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

constexpr int kD2DecoderNum = 4;
constexpr int kD2Modules[kD2DecoderNum] = {4, 5, 6, 7};
constexpr int kD2Rates[kD2DecoderNum] = {100, 100, 100, 250};

bool MapT0d2Event(const glimmer::DecodeEvent &decode, glimmer::RawDssdEvent &dssd) {
	// Placeholder mapping copied as framework only.
	switch (decode.module) {
		case 4:
			if (decode.channel == 15) return false;
		case 5:
			dssd.side = glimmer::kDssdSideFront;
			dssd.strip = decode.channel;
			break;
		case 6:
			if (decode.channel == 1) {
				dssd.side = glimmer::kDssdSideFront;
				dssd.strip = 15;
			} else {
				dssd.side = glimmer::kDssdSideBack;
				dssd.strip = decode.channel;
			}
			break;
		case 7:
			decode.side = glimmer::kDssdSideBack;
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

} // namespace

void DecodeT0d2(
	const char *raw_path,
	const char *raw_prefix,
	const char *workspace,
	int run,
	bool report
) {
	TString filename = TString::Format("%s/decode/t0d1_%04d.root", workspace, run);
	TFile opf(filename, "recreate");
	TTree opt("tree", "decode");
	glimmer::RawDssdEvent dssd;
	glimmer::SetupOutput(&opt, dssd);

	if (report) {
		printf("Decoding T0D1   0%%");
		fflush(stdout);
	}
	std::vector<int> module(kD2Modules, kD2Modules + kD2DecoderNum);
	std::vector<int> rate(kD2Rates, kD2Rates + kD2DecoderNum);
	glimmer::Decoder decoder(kD2DecoderNum, run, module, rate, raw_path, raw_prefix);
	for (
		glimmer::DecodeEvent *event = decoder.GetEvent(report);
		event;
		event = decoder.GetEvent(report)
	) {
		if (MapT0d2Event(*event, dssd)) {
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
	std::string raw_path = GetRequired(tbl, "raw_xia1_path");
	std::string raw_prefix = GetRequired(tbl, "raw_xia1_prefix");
	std::string workspace = GetRequired(tbl, "workspace");

	DecodeT0d2(
		raw_path.c_str(),
		raw_prefix.c_str(),
		workspace.c_str(),
		run,
		true
	);
	return 0;
}
