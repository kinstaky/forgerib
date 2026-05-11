#include <iostream>

#include <TString.h>

#include "include/forge/colossus.h"
#include "include/forge/forge_ppac.h"
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
			"Detector to forge: ppac",
			cxxopts::value<std::vector<std::string>>()
		);
	options.parse_positional({"detectors"});
	auto result = options.parse(argc, argv);

	bool use_trigger = result["trigger"].as<bool>();
	int run = result["run"].as<int>();
	std::vector<std::string> detectors =
		result["detectors"].as<std::vector<std::string>>();
	if (result.count("help") || !result.count("run") || detectors.empty()) {
		std::cout << options.help() << std::endl;
		return 0;
	}

	toml::table tbl = toml::parse_file("config.toml");
	if (!tbl.contains("workspace") || !tbl["workspace"].is_string()) {
		std::cerr << "Error: Missing required parameter workspace from config.toml.\n";
		return -1;
	}
	std::string workspace = tbl["workspace"].as_string()->get();

	std::vector<double> triggers;
	if (use_trigger) {
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
		}
	}

	return 0;
}