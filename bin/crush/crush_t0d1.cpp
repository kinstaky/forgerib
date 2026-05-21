#include "include/crush/crusher.h"

#include <iostream>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "include/config.h"
#include "include/util.h"
#include "include/event/ore/raw_dssd_event.h"

namespace {

constexpr int kT0d1CrusherNum = 4;
constexpr int kT0d1Modules[kT0d1CrusherNum] = {4, 5, 6, 7};
constexpr int kT0d1Rates[kT0d1CrusherNum] = {100, 100, 100, 250};

bool MapT0d1Event(const forgerib::DecodeEvent &decode, forgerib::RawDssdEvent &dssd) {
	switch (decode.module) {
	case 4:
		if (decode.channel == 15) return false;
		[[fallthrough]];
	case 5:
		dssd.side = forgerib::kDssdSideFront;
		dssd.strip = decode.channel + (decode.module - 4) * 16;
		break;
	case 6:
		if (decode.channel == 1) {
			dssd.side = forgerib::kDssdSideFront;
			dssd.strip = 15;
		} else {
			dssd.side = forgerib::kDssdSideBack;
			dssd.strip = decode.channel;
		}
		break;
	case 7:
		dssd.side = forgerib::kDssdSideBack;
		dssd.strip = decode.channel + 16;
		break;
	default:
		return false;
	}
	dssd.energy = decode.energy;
	dssd.time = decode.time;
	dssd.cv = decode.cfd_valid;
	return true;
}

int CrushT0d1(
	const char *raw_path,
	const char *raw_prefix,
	const char *output_path,
	int run,
	bool report
) {
	TString filename = TString::Format("%s/t0d1_%04d.root", output_path, run);
	TFile opf(filename, "recreate");
	TTree opt("tree", "crush");
	forgerib::RawDssdEvent dssd;
	forgerib::SetupOutput(&opt, dssd);

	if (report) {
		printf("Crushing T0D1   0%%");
		fflush(stdout);
	}
	std::vector<int> module(kT0d1Modules, kT0d1Modules + kT0d1CrusherNum);
	std::vector<int> rate(kT0d1Rates, kT0d1Rates + kT0d1CrusherNum);
	forgerib::Crusher crusher(kT0d1CrusherNum, run, 1, module, rate, raw_path, raw_prefix);
	for (
		forgerib::DecodeEvent *event = crusher.GetEvent(report);
		event;
		event = crusher.GetEvent(report)
	) {
		if (MapT0d1Event(*event, dssd)) {
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
	return CrushT0d1(
		config.paths.raw_xia1.c_str(),
		config.prefix.raw_xia1.c_str(),
		output_dir.c_str(),
		run,
		true
	);
}
