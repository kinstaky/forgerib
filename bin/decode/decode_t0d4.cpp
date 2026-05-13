#include "include/decode/decoder.h"

#include <iostream>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "external/toml.hpp"
#include "include/event/raw/raw_dssd_event.h"

namespace {

std::string GetRequired(const toml::table &tbl, const char *key) {
	if (!tbl.contains(key) || !tbl[key].is_string()) {
		std::cerr << "Error: Missing required parameter: " << key << "\n";
		exit(-1);
	}
	return tbl[key].as_string()->get();
}

constexpr int kD4DecoderNum = 5;
constexpr int kD4Modules[kD4DecoderNum] = {1, 2, 3, 4, 6};
constexpr int kD4Rates[kD4DecoderNum] = {100, 100, 100, 100, 100};

bool MapT0d4Event(const glimmer::DecodeEvent &decode, glimmer::RawDssdEvent &dssd) {
	if (decode.module == 1) {
		if (decode.channel >= 12) return false;
		dssd.side= glimmer::kDssdSideFront;
		dssd.strip = decode.channel;
	} else if (decode.module == 2) {
		if (decode.channel == 0) return false;
		dssd.side = glimmer::kDssdSideFront;
		dssd.strip = (decode.module - 1) * 16 + decode.channel;
	} else if (decode.module == 3 || decode.module == 4) {
		dssd.side = glimmer::kDssdSideBack;
		dssd.strip = (decode.module - 3) * 16 + decode.channel;
	} else if (decode.module == 6) {
		dssd.side = glimmer::kDssdSideFront;
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

} // namespace

void DecodeT0d4(
	const char *raw_path,
	const char *raw_prefix,
	const char *workspace,
	int run,
	bool report
) {
	TString filename = TString::Format("%s/decode/t0d4_%04d.root", workspace, run);
	TFile opf(filename, "recreate");
	TTree opt("tree", "decode");
	glimmer::RawDssdEvent dssd;
	glimmer::SetupOutput(&opt, dssd);

	if (report) {
		printf("Decoding T0D4   0%%");
		fflush(stdout);
	}
	std::vector<int> module(kD4Modules, kD4Modules + kD4DecoderNum);
	std::vector<int> rate(kD4Rates, kD4Rates + kD4DecoderNum);
	glimmer::Decoder decoder(kD4DecoderNum, run, module, rate, raw_path, raw_prefix);
	for (
		glimmer::DecodeEvent *event = decoder.GetEvent(report);
		event;
		event = decoder.GetEvent(report)
	) {
		if (MapT0d4Event(*event, dssd)) {
			opt.Fill();
		}
	}
	if (report) printf("\b\b\b\b100%%\n");

	opf.cd();
	opt.Write();
	opf.Close();
}

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " [run].\n";
		return 1;
	}
	const int run = atoi(argv[1]);

	toml::table tbl = toml::parse_file("config.toml");
	std::string raw_path = GetRequired(tbl, "raw_xia2_path");
	std::string raw_prefix = GetRequired(tbl, "raw_xia2_prefix");
	std::string workspace = GetRequired(tbl, "workspace");

	DecodeT0d4(
		raw_path.c_str(),
		raw_prefix.c_str(),
		workspace.c_str(),
		run,
		true
	);
	return 0;
}
