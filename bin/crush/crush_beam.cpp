#include "include/crush/crusher.h"

#include <iostream>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "include/config.h"
#include "include/crush/util.h"
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
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " [run].\n";
		return 1;
	}
	const int run = atoi(argv[1]);

	forgerib::AppConfig config;
	if (forgerib::LoadConfig("config.toml", config)) {
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
