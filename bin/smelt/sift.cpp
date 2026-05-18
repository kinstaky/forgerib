#include <iostream>
#include <map>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "external/cxxopts.hpp"

#include "include/config.h"
#include "include/crush/util.h"
#include "include/event/ingot/trigger_event.h"
#include "include/smelt/sifting.h"

using namespace forgerib;

int ReadXiaTimes(
	const char *path,
	std::vector<long long> &times,
	std::vector<long long> &entries,
	bool external,
	bool report = false
) {
	TFile ipf(path, "read");
	TTree *ipt = static_cast<TTree*>(ipf.Get("tree"));
	if (!ipt) {
		std::cerr << "Error: Get tree from " << path << " failed.\n";
		return -1;
	}
	forgerib::TriggerEvent trigger_event;
	forgerib::SetupInput(ipt, trigger_event);

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
				: static_cast<long long>(trigger_event.time[1])
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
	TTree *ipt = static_cast<TTree*>(ipf.Get("tree"));
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
		if (!valid || time == 0) continue;
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

int main(int argc, char **argv) {
	cxxopts::Options options("sift", "sift trigger alignment");
	options.add_options()
		("h,help", "Print usage")
		(
			"x,xia_run",
			"XIA run number, required.",
			cxxopts::value<int>(),
			"run"
		)
		(
			"v,vme_run",
			"VME run number, required.",
			cxxopts::value<int>(),
			"run"
		)
		(
			"e,external",
			"Use XIA's external timestamp.",
			cxxopts::value<bool>()->default_value("false")
		)
		(
			"t,trigger",
			"VME trigger source: taf or t1. Omit to read all VME sdc.",
			cxxopts::value<std::string>()->default_value(""),
			"trigger"
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

	AppConfig config;
	if (LoadConfig("config.toml", config)) {
		return -1;
	}
	const std::string grit_dir = JoinPath(config.workspace, config.paths.grit);
	const std::string grain_dir = JoinPath(config.workspace, config.paths.grain);
	const std::string ingot_dir = JoinPath(config.workspace, config.paths.ingot);

	std::vector<long long> xia_times;
	std::vector<long long> xia_entries;
	TString xia_filename = TString::Format(
		"%s/trigger_%04d.root",
		ingot_dir.c_str(),
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
		"%s/trigger_vme_%s%04d.root",
		grit_dir.c_str(),
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
		"%s/grain_%s%04d.root",
		grain_dir.c_str(),
		trigger.empty() ? "" : (trigger+"_").c_str(),
		xia_run
	);

	std::map<int, int> group_num_table = {
		{90, 30},
		{160, 1000}
	};
	int group_num = 300;
	auto found = group_num_table.find(xia_run);
	if (found != group_num_table.end()) {
		group_num = found->second;
	}

	double window = 3'000'000;
	std::map<int, double> window_table = {
		{160, 100'000}
	};
	auto found_window = window_table.find(xia_run);
	if (found_window != window_table.end()) {
		window = found_window->second;
	}
	int result = Sift(
		xia_times,
		xia_entries,
		vme_times,
		vme_entries,
		output_file,
		group_num,
		window,
		-10'000'000'000,
		10'000'000'000,
		true,
		true
	);

	if (result) {
		std::cerr << "Error: Sift failed.\n";
		return -1;
	}
	return 0;
}
