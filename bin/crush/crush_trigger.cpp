#include <iostream>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "external/cxxopts.hpp"
#include "include/config.h"
#include "include/crush/raw_reader.h"
#include "include/util.h"
#include "include/event/ore/decode_event.h"
#include "include/event/ore/raw_trigger_event.h"

namespace {

int CrushTrigger(
	const char *raw_path,
	const char *raw_prefix,
	const char *output_path,
	int run,
	bool report
) {
	TString filename = TString::Format("%s/trigger_%04d.root", output_path, run);
	TFile opf(filename, "recreate");
	TTree opt("tree", "crush");
	forgerib::RawTriggerEvent trigger;
	forgerib::SetupOutput(&opt, trigger);

	if (report) {
		printf("Crushing trigger   0%%");
		fflush(stdout);
	}
	size_t last_percentage = 0;

	forgerib::RawReader reader(raw_path, raw_prefix, run, 0, 0, 250);
	forgerib::DecodeEvent event;
	forgerib::RawExternalTime *external_time = nullptr;

	size_t total_bytes = reader.FileSize();
	size_t bytes = 0;
	while (bytes < total_bytes) {
		if (report && total_bytes > 0 && bytes * 100lu / total_bytes > last_percentage) {
			last_percentage = bytes * 100lu / total_bytes;
			printf("\b\b\b\b%3lu%%", last_percentage);
			fflush(stdout);
		}
		size_t read_bytes = reader.Read(&event, nullptr, nullptr, &external_time);
		if (read_bytes >= 16) bytes += read_bytes;
		else if (read_bytes == 0) break;
		if (event.channel >= 4 && event.channel <= 9 && external_time) {
			trigger.type = event.channel - 4;
			trigger.time = event.time;
			trigger.external_time =
				((long long)(external_time->high) << 32)
				| ((long long)(external_time->low));
			trigger.cv = event.cfd_valid;
			opt.Fill();
		}
	}
	if (report) printf("\b\b\b\b100%%\n");

	opf.cd();
	opt.Write();
	opf.Close();
	return 0;
}

} // namespace

int main(int argc, char **argv) {
	cxxopts::Options options("crush_trigger", "crush trigger ROOT output");
	options.add_options()
		("h,help", "Print usage")
		(
			"c,config",
			"Config file path.",
			cxxopts::value<std::string>()->default_value("config.toml"),
			"path"
		)
		(
			"run",
			"Run number.",
			cxxopts::value<int>(),
			"run"
		);
	options.parse_positional({"run"});
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
	const std::string output_dir = forgerib::JoinPath(
		config.workspace,
		config.paths.grit
	);
	return CrushTrigger(
		config.paths.raw_xia0.c_str(),
		config.prefix.raw_xia0.c_str(),
		output_dir.c_str(),
		run,
		true
	);
}
