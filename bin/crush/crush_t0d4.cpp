#include "include/crush/crusher.h"

#include <iostream>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "include/config.h"
#include "include/crush/util.h"
#include "include/event/ore/raw_dssd_event.h"

namespace {

constexpr int kT0d4CrusherNum = 5;
constexpr int kT0d4Modules[kT0d4CrusherNum] = {1, 2, 3, 4, 6};
constexpr int kT0d4Rates[kT0d4CrusherNum] = {100, 100, 100, 100, 100};

bool MapT0d4Event(const forgerib::DecodeEvent &decode, forgerib::RawDssdEvent &dssd) {
	if (decode.module == 1) {
		if (decode.channel >= 12) return false;
		dssd.side = forgerib::kDssdSideFront;
		dssd.strip = decode.channel;
	} else if (decode.module == 2) {
		if (decode.channel == 0) return false;
		dssd.side = forgerib::kDssdSideFront;
		dssd.strip = (decode.module - 1) * 16 + decode.channel;
	} else if (decode.module == 3) {
		dssd.side = forgerib::kDssdSideBack;
		dssd.strip = decode.channel + 16;
	} else if (decode.module == 4) {
		dssd.side = forgerib::kDssdSideBack;
		dssd.strip = decode.channel;
	} else if (decode.module == 6) {
		dssd.side = forgerib::kDssdSideFront;
		if (decode.channel == 7) {
			dssd.strip = 16;
		} else if (decode.channel >= 12 && decode.channel <= 15) {
			dssd.strip = decode.channel;
		} else {
			return false;
		}
	} else {
		return false;
	}
	dssd.energy = decode.energy;
	dssd.time = decode.time;
	dssd.cv = decode.cfd_valid;
	return true;
}

int CrushT0d4(
	const char *raw_path,
	const char *raw_prefix,
	const char *output_path,
	int run,
	bool report
) {
	TString filename = TString::Format("%s/t0d4_%04d.root", output_path, run);
	TFile opf(filename, "recreate");
	TTree opt("tree", "crush");
	forgerib::RawDssdEvent dssd;
	forgerib::SetupOutput(&opt, dssd);

	if (report) {
		printf("Crushing T0D4   0%%");
		fflush(stdout);
	}
	std::vector<int> module(kT0d4Modules, kT0d4Modules + kT0d4CrusherNum);
	std::vector<int> rate(kT0d4Rates, kT0d4Rates + kT0d4CrusherNum);
	forgerib::Crusher crusher(kT0d4CrusherNum, run, 2, module, rate, raw_path, raw_prefix);
	for (
		forgerib::DecodeEvent *event = crusher.GetEvent(report);
		event;
		event = crusher.GetEvent(report)
	) {
		if (MapT0d4Event(*event, dssd)) {
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
	return CrushT0d4(
		config.paths.raw_xia2.c_str(),
		config.prefix.raw_xia2.c_str(),
		output_dir.c_str(),
		run,
		true
	);
}
