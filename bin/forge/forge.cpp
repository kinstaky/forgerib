#include <iostream>

#include <TString.h>

#include "include/forge/colossus.h"
#include "include/forge/forge_beam.h"
#include "include/forge/forge_t0csi.h"
#include "include/forge/forge_ppac.h"
#include "include/forge/forge_ssd.h"
#include "include/forge/forge_t0d1.h"
#include "include/forge/forge_t0d2.h"
#include "include/forge/forge_t0d3.h"
#include "include/forge/forge_t0d4.h"
#include "include/forge/forge_trigger.h"
#include "include/event/raw/raw_trigger_event.h"

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


int main(int argc, char **argv) {

	// TString trigger_path = TString::Format(
	// 	"/data/xia0/test/decode/online_R%04d.root",
	// 	74
	// );
	// TString vme_path = TString::Format(
	// 	"/data/vme/test/root_file/data%04d.root",
	// 	210
	// );
	// TString output_path = TString::Format(
	// 	"/data/glimmer/forge/test-case-1.root"
	// );
	// ForgeVme(
	// 	trigger_path.Data(),
	// 	vme_path.Data(),
	// 	output_path.Data(),
	// 	1000,
	// 	500
	// );

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
			"detectors",
			"Detector to forge: ppac, t0d1, t0d2, t0d3, t0d4, t0s, t1su, t1sd, t0csi, beam, trigger",
			cxxopts::value<std::vector<std::string>>()
		);
	options.parse_positional({"detectors"});
	auto result = options.parse(argc, argv);

	if (result.count("help") || !result.count("run")) {
		std::cout << options.help() << std::endl;
		return 0;
	}
	bool use_trigger = result["trigger"].as<bool>();
	int run = result["run"].as<int>();
	std::vector<std::string> detectors =
		result["detectors"].as<std::vector<std::string>>();
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
	for (const std::string &det : detectors) {
		if (det == "trigger") {
			need_trigger = true;
			break;
		}
	}

	std::vector<double> triggers;
	if (need_trigger) {
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
		}
	}

	return 0;
}
