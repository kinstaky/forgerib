#include <iostream>

#include <TString.h>
#include <TFile.h>
#include <TTree.h>

#include "external/cxxopts.hpp"
#include "external/toml.hpp"

#include "include/event/forge/trigger_event.h"
#include "include/forge/alignment.h"

using namespace glimmer;

int ReadXiaTimes(
	const char *path,
	std::vector<long long> &times,
	std::vector<long long> &entries,
	bool external,
	bool report = false
) {
	TFile ipf(path, "read");
	TTree *ipt = (TTree*)ipf.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from "
			<< path << " failed.\n";
		return -1;
	}
	glimmer::TriggerEvent trigger_event;
	glimmer::SetupInput(ipt, trigger_event);

	times.clear();
	entries.clear();
	if (report) {
		printf("Reading XIA times   0%%");
		fflush(stdout);
	}
	long long total = ipt->GetEntries();
	long long last_percentage = 0;
	for (long long entry = 0; entry < total; ++entry) {
		if (report && entry * 100ll / total > last_percentage) {
			last_percentage = entry * 100ll / total;
			printf("\b\b\b\b%3lld%%", last_percentage);
			fflush(stdout);
		}
		ipt->GetEntry(entry);
		if (trigger_event.valid[1]) {
			times.push_back(
				external
				? trigger_event.external_time[1] * 200
				: (long long)trigger_event.time[1]
			);
			entries.push_back(entry);
		}
	}
	if (report) {
		printf("\b\b\b\b100%%\n");
	}
	return 0;
}


int ReadVmeTriggerTimes(
	const char *path,
	std::vector<long long> &times,
	std::vector<long long> &entries,
	bool report = false
) {
	TFile ipf(path, "read");
	TTree *ipt = (TTree*)ipf.Get("tree");
	if (!ipt) {
		std::cerr << "Error: get tree from " << path << " failed.\n";
		return -1;
	}

	bool valid = false;
	long long time = 0;
	ipt->SetBranchAddress("valid", &valid);
	ipt->SetBranchAddress("time", &time);

	times.clear();
	entries.clear();
	long long bit_flip_offset = 0;
	long long last_timestamp = 0;
	long long total = ipt->GetEntries();
	long long last_percentage = 0;
	if (report) {
		printf("Reading VME trigger times   0%%");
		fflush(stdout);
	}
	for (long long entry = 0; entry < total; ++entry) {
		if (report && entry * 100ll / total > last_percentage) {
			last_percentage = entry * 100ll / total;
			printf("\b\b\b\b%3lld%%", last_percentage);
			fflush(stdout);
		}

		ipt->GetEntry(entry);
		if (!valid) continue;
		if (time == 0) continue;
		long long timestamp = (time + bit_flip_offset) * 200;
		if (timestamp < last_timestamp) {
			bit_flip_offset += 1ll << 32;
			timestamp += (1ll << 32) * 200;
		}
		last_timestamp = timestamp;

		times.push_back(timestamp);
		entries.push_back(entry);
	}
	if (report) printf("\b\b\b\b100%%\n");
	ipf.Close();
	return 0;
}

std::string RequireKey(const toml::table &tbl, const char *key) {
	if (!tbl.contains(key) || !tbl[key].is_string()) {
		std::cerr << "Error: Missing required parameter "
			<< key << " from config.toml.\n";
		exit(-1);
	}
	return tbl[key].as_string()->get();
}

int main(int argc, char **argv) {
	cxxopts::Options options("forge", "forge detectors");
	options.add_options()
		("h,help", "Print usage")
		(
			"x,xia_run",
			"XIA run number, required.",
			cxxopts::value<int>()
		)
		(
			"v,vme_run",
			"VME run number, required.",
			cxxopts::value<int>()
		)
		(
			"e,external",
			"Use XIA's external timestamp.",
			cxxopts::value<bool>()->default_value("false")
		)
		(
			"t,trigger",
			"VME trigger source: taf or t1. Omit to read all VME sdc.",
			cxxopts::value<std::string>()->default_value("")
		);
	auto parse_result = options.parse(argc, argv);

	if (
		parse_result.count("help")
		|| !parse_result.count("xia_run")
		|| !parse_result.count("vme_run")
	) {
		std::cout << options.help() << std::endl;
		return 0;
	}
	int xia_run = parse_result["xia_run"].as<int>();
	int vme_run = parse_result["vme_run"].as<int>();
	bool external = parse_result["external"].as<bool>();
	std::string trigger = parse_result["trigger"].as<std::string>();
	if (trigger != "" && trigger != "taf" && trigger != "t1") {
		std::cerr << "Error: Unsupported trigger option " << trigger
			<< ", use taf or t1.\n";
		return -1;
	}

	toml::table tbl = toml::parse_file("config.toml");
	std::string workspace = RequireKey(tbl, "workspace");
	std::string vme_path = RequireKey(tbl, "vme_path");
	std::string vme_prefix = RequireKey(tbl, "vme_prefix");

	std::vector<long long> xia_times;
	std::vector<long long> xia_entries;
	TString xia_filename = TString::Format(
		"%s/forge/trigger_%04d.root",
		workspace.c_str(),
		xia_run
	);
	if (ReadXiaTimes(
		xia_filename.Data(),
		xia_times,
		xia_entries,
		external,
		true
	)) {
		std::cerr << "Error: Read XIA times failed.\n";
		return -2;
	}

	std::vector<long long> vme_times;
	std::vector<long long> vme_entries;
	TString vme_trigger_filename = TString::Format(
		"%s/forge/trigger_vme_%s%04d.root",
		workspace.c_str(),
		trigger.empty() ? "" : (trigger+"_").c_str(),
		vme_run
	);
	if (ReadVmeTriggerTimes(
		vme_trigger_filename.Data(),
		vme_times,
		vme_entries,
		true
	)) {
		std::cerr << "Error: Read VME trigger times failed.\n";
		return -2;
	}

	TString output_file = TString::Format(
		"%s/forge/align_%s%04d.root",
		workspace.c_str(),
		trigger.empty() ? "" : (trigger+"_").c_str(),
		xia_run
	);
	int group_num = xia_run == 90
		? 10 : 300;
	int result = Align(
		xia_times,
		xia_entries,
		vme_times,
		vme_entries,
		output_file,
		group_num,
		3'000'000,
		-10'000'000'000,
		10'000'000'000,
		true,
		true
	);

	if (result) {
		std::cerr << "Error: Align failed.\n";
		return -1;
	}
	return 0;
}
