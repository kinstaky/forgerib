#include <iostream>

#include <TString.h>

#include "include/forge/forge_beam.h"
#include "include/forge/forge_t0csi.h"
#include "include/forge/forge_ppac.h"
#include "include/forge/forge_ssd.h"
#include "include/forge/forge_t0d1.h"
#include "include/forge/forge_t0d2.h"
#include "include/forge/forge_t0d3.h"
#include "include/forge/forge_t0d4.h"
#include "include/forge/forge_trigger.h"
#include "include/forge/forge_vme.h"
#include "include/event/raw/raw_trigger_event.h"
#include "include/event/forge/trigger_event.h"

#include "external/cxxopts.hpp"
#include "external/toml.hpp"

using namespace glimmer;

int ReadMainTrigger(
	const char *workspace,
	int run,
	std::vector<double> &trigger,
	bool report = false
) {
	TString filename = TString::Format(
		"%s/decode/trigger_%04d.root",
		workspace,
		run
	);
	TFile ipf(filename, "read");
	TTree *ipt = (TTree*)ipf.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from "
			<< filename << " failed.\n";
		return -1;
	}
	glimmer::RawTriggerEvent trigger_event;
	glimmer::SetupInput(ipt, trigger_event);

	trigger.clear();
	if (report) {
		printf("Reading trigger   0%%");
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
		if (trigger_event.type != glimmer::kTriggerMain) continue;
		trigger.push_back(trigger_event.time);
	}
	if (report) {
		printf("\b\b\b\b100%%\n");
	}
	return 0;
}

int ReadVmeTriggerWithMainEntries(
	const char *workspace,
	int run,
	std::vector<long long> &trigger,
	std::vector<long long> &entries,
	bool report = false
) {
	TString filename = TString::Format(
		"%s/forge/trigger_%04d.root",
		workspace,
		run
	);
	TFile ipf(filename, "read");
	TTree *ipt = (TTree*)ipf.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from "
			<< filename << " failed.\n";
		return -1;
	}
	glimmer::TriggerEvent trigger_event;
	glimmer::SetupInput(ipt, trigger_event);

	trigger.clear();
	if (report) {
		printf("Reading trigger   0%%");
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
		if (trigger_event.valid[kTriggerVme]) {
			trigger.push_back(trigger_event.external_time[kTriggerVme]);
			entries.push_back(entry);
		}
	}
	if (report) {
		printf("\b\b\b\b100%%\n");
	}
	return 0;
}


int main(int argc, char **argv) {
	cxxopts::Options options("forge", "forge detectors");
	options.add_options()
		("h,help", "Print usage")
		(
			"t,trigger",
			"Use main trigger.",
			cxxopts::value<bool>()->default_value("false")
		)
		(
			"r,run",
			"Run number, required.",
			cxxopts::value<int>()
		)
		(
			"v,vme_run",
			"VME run number.",
			cxxopts::value<int>()
		)
		(
			"detectors",
			"Detector to forge: ppac, t0d1, t0d2, t0d3, t0d4, t0s, t1su, t1sd, t0csi, beam, trigger, vme",
			cxxopts::value<std::vector<std::string>>()
		);
	options.parse_positional({"detectors"});
	auto parse_result = options.parse(argc, argv);

	if (parse_result.count("help") || !parse_result.count("run")) {
		std::cout << options.help() << std::endl;
		return 0;
	}
	bool use_trigger = parse_result["trigger"].as<bool>();
	int run = parse_result["run"].as<int>();
	std::vector<std::string> detectors =
		parse_result["detectors"].as<std::vector<std::string>>();
	if (detectors.empty()) {
		std::cout << options.help() << std::endl;
		return 0;
	}

	toml::table tbl = toml::parse_file("config.toml");
	if (!tbl.contains("workspace") || !tbl["workspace"].is_string()) {
		std::cerr << "Error: Missing required parameter workspace from config.toml.\n";
		return -1;
	}
	std::string workspace = tbl["workspace"].as_string()->get();

	bool need_trigger = use_trigger;

	std::vector<double> triggers;
	if (need_trigger && (detectors.size() > 1 || detectors[0] != "vme")) {
		if (ReadMainTrigger(
			workspace.c_str(),
			run,
			triggers,
			true
		)) {
			std::cerr << "Error: Read main trigger failed.\n";
			return -1;
		}
	}

	for (const std::string &det : detectors) {
		if (det == "ppac") {
			TString input_path = TString::Format(
				"%s/decode/ppac_%04d.root",
				workspace.c_str(),
				run
			);
			TString output_path = TString::Format(
				"%s/forge/ppac_%s%04d.root",
				workspace.c_str(),
				use_trigger ? "" : "nt_",
				run
			);
			int result = glimmer::ForgePpac(
				triggers,
				input_path.Data(),
				output_path.Data(),
				1000.0,
				true
			);
			if (result) {
				std::cerr << "Error: Froge ppac failed.\n";
			} else {
				std::cout << "Forge ppac success!\n";
			}
		} else if (det == "t0d1") {
			TString input_path = TString::Format(
				"%s/decode/t0d1_%04d.root",
				workspace.c_str(),
				run
			);
			TString output_path = TString::Format(
				"%s/forge/t0d1_%s%04d.root",
				workspace.c_str(),
				use_trigger ? "" : "nt_",
				run
			);
			int result = glimmer::ForgeT0d1(
				triggers,
				input_path.Data(),
				output_path.Data(),
				1000.0,
				true
			);
			if (result) {
				std::cerr << "Error: Froge t0d1 failed.\n";
			} else {
				std::cout << "Forge t0d1 success!\n";
			}
		} else if (det == "t0d2") {
			TString input_path = TString::Format(
				"%s/decode/t0d2_%04d.root",
				workspace.c_str(),
				run
			);
			TString output_path = TString::Format(
				"%s/forge/t0d2_%s%04d.root",
				workspace.c_str(),
				use_trigger ? "" : "nt_",
				run
			);
			int result = glimmer::ForgeT0d2(
				triggers,
				input_path.Data(),
				output_path.Data(),
				1000.0,
				true
			);
			if (result) {
				std::cerr << "Error: Froge t0d2 failed.\n";
			} else {
				std::cout << "Forge t0d2 success!\n";
			}
		} else if (det == "t0d3") {
			TString input_path = TString::Format(
				"%s/decode/t0d3_%04d.root",
				workspace.c_str(),
				run
			);
			TString output_path = TString::Format(
				"%s/forge/t0d3_%s%04d.root",
				workspace.c_str(),
				use_trigger ? "" : "nt_",
				run
			);
			int result = glimmer::ForgeT0d3(
				triggers,
				input_path.Data(),
				output_path.Data(),
				1000.0,
				true
			);
			if (result) {
				std::cerr << "Error: Froge t0d3 failed.\n";
			} else {
				std::cout << "Forge t0d3 success!\n";
			}
		} else if (det == "t0d4") {
			TString input_path = TString::Format(
				"%s/decode/t0d4_%04d.root",
				workspace.c_str(),
				run
			);
			TString output_path = TString::Format(
				"%s/forge/t0d4_%s%04d.root",
				workspace.c_str(),
				use_trigger ? "" : "nt_",
				run
			);
			int result = glimmer::ForgeT0d4(
				triggers,
				input_path.Data(),
				output_path.Data(),
				1000.0,
				true
			);
			if (result) {
				std::cerr << "Error: Froge t0d4 failed.\n";
			} else {
				std::cout << "Forge t0d4 success!\n";
			}
		} else if (det == "t0s") {
			TString input_path = TString::Format(
				"%s/decode/t0s_%04d.root",
				workspace.c_str(),
				run
			);
			TString output_path = TString::Format(
				"%s/forge/t0s_%s%04d.root",
				workspace.c_str(),
				use_trigger ? "" : "nt_",
				run
			);
			int result = glimmer::ForgeT0s(
				triggers,
				input_path.Data(),
				output_path.Data(),
				1000.0,
				true
			);
			if (result) {
				std::cerr << "Error: Froge t0s failed.\n";
			} else {
				std::cout << "Forge t0s success!\n";
			}
		} else if (det == "t1su") {
			TString input_path = TString::Format(
				"%s/decode/t1su_%04d.root",
				workspace.c_str(),
				run
			);
			TString output_path = TString::Format(
				"%s/forge/t1su_%s%04d.root",
				workspace.c_str(),
				use_trigger ? "" : "nt_",
				run
			);
			int result = glimmer::ForgeT1su(
				triggers,
				input_path.Data(),
				output_path.Data(),
				1000.0,
				true
			);
			if (result) {
				std::cerr << "Error: Froge t1su failed.\n";
			} else {
				std::cout << "Forge t1su success!\n";
			}
		} else if (det == "t1sd") {
			TString input_path = TString::Format(
				"%s/decode/t1sd_%04d.root",
				workspace.c_str(),
				run
			);
			TString output_path = TString::Format(
				"%s/forge/t1sd_%s%04d.root",
				workspace.c_str(),
				use_trigger ? "" : "nt_",
				run
			);
			int result = glimmer::ForgeT1sd(
				triggers,
				input_path.Data(),
				output_path.Data(),
				1000.0,
				true
			);
			if (result) {
				std::cerr << "Error: Froge t1sd failed.\n";
			} else {
				std::cout << "Forge t1sd success!\n";
			}
		} else if (det == "t0csi") {
			TString input_path = TString::Format(
				"%s/decode/t0csi_%04d.root",
				workspace.c_str(),
				run
			);
			TString output_path = TString::Format(
				"%s/forge/t0csi_%s%04d.root",
				workspace.c_str(),
				use_trigger ? "" : "nt_",
				run
			);
			int result = glimmer::ForgeT0Csi(
				triggers,
				input_path.Data(),
				output_path.Data(),
				1000.0,
				true
			);
			if (result) {
				std::cerr << "Error: Froge t0csi failed.\n";
			} else {
				std::cout << "Forge t0csi success!\n";
			}
		} else if (det == "beam") {
			TString input_path = TString::Format(
				"%s/decode/beam_%04d.root",
				workspace.c_str(),
				run
			);
			TString output_path = TString::Format(
				"%s/forge/beam_%s%04d.root",
				workspace.c_str(),
				use_trigger ? "" : "nt_",
				run
			);
			int result = glimmer::ForgeBeam(
				triggers,
				input_path.Data(),
				output_path.Data(),
				1000.0,
				true
			);
			if (result) {
				std::cerr << "Error: Froge beam failed.\n";
			} else {
				std::cout << "Forge beam success!\n";
			}
		} else if (det == "trigger") {
			TString input_path = TString::Format(
				"%s/decode/trigger_%04d.root",
				workspace.c_str(),
				run
			);
			TString output_path = TString::Format(
				"%s/forge/trigger_%04d.root",
				workspace.c_str(),
				run
			);
			int result = glimmer::ForgeTrigger(
				triggers,
				input_path.Data(),
				output_path.Data(),
				1000.0,
				true
			);
			if (result) {
				std::cerr << "Error: Froge trigger failed.\n";
			} else {
				std::cout << "Forge trigger success!\n";
			}
		} else if (det == "vme") {
			if (!tbl.contains("vme_path") || !tbl["vme_path"].is_string()) {
				std::cerr << "Error: Missing reqruied parameter vme_path from config.toml.\n";
				continue;
			}
			if (!tbl.contains("vme_prefix") || !tbl["vme_prefix"].is_string()) {
				std::cerr << "Error: Missing reqruied parameter vme_prefix from config.toml.\n";
				continue;
			}
			std::string vme_path = tbl["vme_path"].as_string()->get();
			std::string vme_prefix = tbl["vme_prefix"].as_string()->get();
			if (!parse_result.count("vme_run")) {
				std::cerr << "Error: Missing required args vme_run, use -h for help.\n";
				continue;
			}
			int vme_run = parse_result["vme_run"].as<int>();
			std::vector<long long> vme_triggers;
			std::vector<long long> trigger_entries;
			if (ReadVmeTriggerWithMainEntries(
				workspace.c_str(),
				run,
				vme_triggers,
				trigger_entries,
				true
			)) {
				std::cerr << "Error: Read vme trigger with entries failed.\n";
				return -1;
			}
			TString input_path = TString::Format(
				"%s/%s%04d.root",
				vme_path.c_str(),
				vme_prefix.c_str(),
				vme_run
			);
			TString output_path = TString::Format(
				"%s/forge/vme_%04d.root",
				workspace.c_str(),
				run
			);
			int result = glimmer::ForgeVmeWithTrigger(
				vme_triggers,
				trigger_entries,
				input_path.Data(),
				output_path.Data(),
				1000.0,
				10000,
				true
			);
			if (result) {
				std::cerr << "Error: Froge VME with trigger failed.\n";
			} else {
				std::cout << "Forge trigger success!\n";
			}
		}
	}

	return 0;
}
