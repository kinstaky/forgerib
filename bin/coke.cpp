#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "external/cxxopts.hpp"
#include "include/coke/coke_trigger.h"
#include "include/config.h"
#include "include/event/ore/raw_trigger_event.h"

namespace {

std::string JoinPath(const std::string &base, const std::string &path) {
	if (path.empty()) return base;
	const std::filesystem::path other(path);
	if (other.is_absolute()) return other.string();
	return (std::filesystem::path(base) / other).string();
}

int ReadMainTrigger(
	const std::string &path,
	int run,
	std::vector<double> &trigger,
	bool report = false
) {
	TString filename = TString::Format("%s/trigger_%04d.root", path.c_str(), run);
	TFile ipf(filename, "read");
	TTree *ipt = static_cast<TTree*>(ipf.Get("tree"));
	if (!ipt) {
		std::cerr << "Error: Get tree from " << filename << " failed.\n";
		return -1;
	}
	forgerib::RawTriggerEvent trigger_event;
	forgerib::SetupInput(ipt, trigger_event);

	trigger.clear();
	if (report) {
		printf("Reading trigger   0%%");
		fflush(stdout);
	}
	long long total = ipt->GetEntries();
	long long last_percentage = 0;
	for (long long entry = 0; entry < total; ++entry) {
		if (report && total > 0 && entry * 100ll / total > last_percentage) {
			last_percentage = entry * 100ll / total;
			printf("\b\b\b\b%3lld%%", last_percentage);
			fflush(stdout);
		}
		ipt->GetEntry(entry);
		if (trigger_event.type != forgerib::kTriggerMain) continue;
		trigger.push_back(trigger_event.time);
	}
	if (report) {
		printf("\b\b\b\b100%%\n");
	}
	ipf.Close();
	return 0;
}

} // namespace

int main(int argc, char **argv) {
	cxxopts::Options options("coke", "coke trigger events into ingot output");
	options.add_options()
		("h,help", "Print usage")
		(
			"r,run",
			"Run number, required.",
			cxxopts::value<int>(),
			"run"
		)
		(
			"c,config",
			"Config file path.",
			cxxopts::value<std::string>()->default_value("config.toml"),
			"path"
		);
	auto parse_result = options.parse(argc, argv);

	if (parse_result.count("help") || !parse_result.count("run")) {
		std::cout << options.help() << "\n";
		return 0;
	}

	const int run = parse_result["run"].as<int>();
	const std::string config_path = parse_result["config"].as<std::string>();

	forgerib::AppConfig config;
	if (forgerib::LoadConfig(config_path, config)) {
		return -1;
	}

	const std::string trigger_path = JoinPath(config.workspace, config.paths.grit);
	std::vector<double> triggers;
	if (ReadMainTrigger(trigger_path, run, triggers, true)) {
		std::cerr << "Error: Read main trigger failed.\n";
		return -1;
	}

	const std::string input_dir = JoinPath(config.workspace, config.paths.grit);
	const std::string output_dir = JoinPath(config.workspace, config.paths.ingot);
	TString input_path = TString::Format("%s/trigger_%04d.root", input_dir.c_str(), run);
	TString output_path = TString::Format("%s/trigger_%04d.root", output_dir.c_str(), run);
	return forgerib::CokeTrigger(
		triggers,
		input_path.Data(),
		output_path.Data(),
		1000.0,
		true
	);
}
