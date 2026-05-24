#include "include/crush/crusher.h"

#include <iostream>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "external/cxxopts.hpp"
#include "include/config.h"
#include "include/util.h"
#include "include/event/ore/raw_beam_event.h"

namespace {

constexpr int kBeamCrusherNum = 1;
constexpr int kBeamModules[kBeamCrusherNum] = {0};
constexpr int kBeamRates[kBeamCrusherNum] = {250};

bool MapBeamEvent(const forgerib::DecodeEvent &decode, forgerib::RawBeamEvent &beam) {
	switch (decode.channel) {
	case 0:
		beam.type = forgerib::kBeamT1;
		break;
	case 1:
		beam.type = forgerib::kBeamT2;
		break;
	case 2:
		beam.type = forgerib::kBeamSi;
		break;
	default:
		return false;
	}
	beam.energy = decode.energy;
	beam.time = decode.time;
	beam.cv = decode.cfd_valid;
	return true;
}

int CrushBeam(
	const char *raw_path,
	const char *raw_prefix,
	const char *output_path,
	int run,
	bool report
) {
	TString filename = TString::Format("%s/beam_%04d.root", output_path, run);
	TFile opf(filename, "recreate");
	TTree opt("tree", "crush");
	forgerib::RawBeamEvent beam;
	forgerib::SetupOutput(&opt, beam);

	if (report) {
		printf("Crushing beam   0%%");
		fflush(stdout);
	}
	std::vector<int> module(kBeamModules, kBeamModules + kBeamCrusherNum);
	std::vector<int> rate(kBeamRates, kBeamRates + kBeamCrusherNum);
	forgerib::Crusher crusher(kBeamCrusherNum, run, 0, module, rate, raw_path, raw_prefix);
	for (
		forgerib::DecodeEvent *event = crusher.GetEvent(report);
		event;
		event = crusher.GetEvent(report)
	) {
		if (MapBeamEvent(*event, beam)) {
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
	cxxopts::Options options("crush_beam", "crush beam detector ROOT output");
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
	return CrushBeam(
		config.paths.raw_xia0.c_str(),
		config.prefix.raw_xia0.c_str(),
		output_dir.c_str(),
		run,
		true
	);
}
