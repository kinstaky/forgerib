#include "include/crush/crusher.h"

#include <iostream>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "external/cxxopts.hpp"
#include "include/config.h"
#include "include/util.h"
#include "include/event/ore/raw_dssd_event.h"

namespace {

constexpr int kT0d3CrusherNum = 4;
constexpr int kT0d3Modules[kT0d3CrusherNum] = {0, 1, 2, 3};
constexpr int kT0d3Rates[kT0d3CrusherNum] = {250, 100, 250, 500};

bool MapT0d3Event(const forgerib::DecodeEvent &decode, forgerib::RawDssdEvent &dssd) {
	if (decode.module == 0 || decode.module == 1) {
		dssd.side = forgerib::kDssdSideFront;
		dssd.strip = decode.module * 16 + decode.channel;
	} else if (decode.module == 2 || decode.module == 3) {
		dssd.side = forgerib::kDssdSideBack;
		dssd.strip = (decode.module - 2) * 16 + decode.channel;
	} else {
		return false;
	}
	dssd.energy = decode.energy;
	dssd.time = decode.time;
	dssd.cv = decode.cfd_valid;
	return true;
}

int CrushT0d3(
	const char *raw_path,
	const char *raw_prefix,
	const char *output_path,
	int run,
	bool report
) {
	TString filename = TString::Format("%s/t0d3_%04d.root", output_path, run);
	TFile opf(filename, "recreate");
	TTree opt("tree", "crush");
	forgerib::RawDssdEvent dssd;
	forgerib::SetupOutput(&opt, dssd);

	if (report) {
		printf("Crushing T0D3   0%%");
		fflush(stdout);
	}
	std::vector<int> module(kT0d3Modules, kT0d3Modules + kT0d3CrusherNum);
	std::vector<int> rate(kT0d3Rates, kT0d3Rates + kT0d3CrusherNum);
	forgerib::Crusher crusher(kT0d3CrusherNum, run, 1, module, rate, raw_path, raw_prefix);
	for (
		forgerib::DecodeEvent *event = crusher.GetEvent(report);
		event;
		event = crusher.GetEvent(report)
	) {
		if (MapT0d3Event(*event, dssd)) {
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
	cxxopts::Options options("crush_t0d3", "crush T0D3 detector ROOT output");
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
	return CrushT0d3(
		config.paths.raw_xia1.c_str(),
		config.prefix.raw_xia1.c_str(),
		output_dir.c_str(),
		run,
		true
	);
}
