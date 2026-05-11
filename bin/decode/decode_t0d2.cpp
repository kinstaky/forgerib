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

constexpr int kD1DecoderNum = 9;
constexpr int kD1Modules[kD1DecoderNum] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
constexpr int kD1Rates[kD1DecoderNum] = {250, 250, 250, 250, 250, 250, 250, 250, 250};

bool MapT0d1Event(const glimmer::DecodeEvent &decode, glimmer::RawDssdEvent &dssd) {
	// Placeholder mapping copied as framework only.
	switch (decode.module) {
		case 0:
			if (decode.channel == 14) {
				dssd.side = glimmer::kDssdSideFront;
				dssd.strip = 15;
				break;
			} else if (decode.channel == 15) {
				dssd.side = glimmer::kDssdSideBack;
				dssd.strip = 16;
				break;
			} else {
				return false;
			}
			break;
		case 1:
			if (decode.channel == 15) return false;
			dssd.side = glimmer::kDssdSideFront;
			dssd.strip = decode.channel;
			break;
		case 2:
		case 3:
		case 4:
			dssd.side = glimmer::kDssdSideFront;
			dssd.strip = decode.channel + (dssd.module - 2) * 16;
			break;
		case 5:
		case 7:
		case 8:
			dssd.side = glimmer::kDssdSideBack;
			dssd.strip = decode.channel + (dssd.module - 5) * 16;
			break;
		case 6:
			if (dssd.channel == 0) return false;
			dssd.side = glimmer::kDssdSideBack;
			dssd.strip = decode.channel;
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

void DecodeT0d1(
	const char *raw_path,
	const char *raw_prefix,
	const char *workspace,
	int run,
	bool report
) {
	TString filename = TString::Format("%s/decode/t0d2_%04d.root", workspace, run);
	TFile opf(filename, "recreate");
	TTree opt("tree", "decode");
	glimmer::RawDssdEvent dssd;
	glimmer::SetupOutput(&opt, dssd);

	if (report) {
		printf("Decoding T0D2   0%%");
		fflush(stdout);
	}
	std::vector<int> module(kD1Modules, kD1Modules + kD1DecoderNum);
	std::vector<int> rate(kD1Rates, kD1Rates + kD1DecoderNum);
	glimmer::Decoder decoder(kD1DecoderNum, run, module, rate, raw_path, raw_prefix);
	for (
		glimmer::DecodeEvent *event = decoder.GetEvent(report);
		event;
		event = decoder.GetEvent(report)
	) {
		if (MapT0d1Event(*event, dssd)) {
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
	std::string raw_path = GetRequired(tbl, "raw_xia0_path");
	std::string raw_prefix = GetRequired(tbl, "raw_xia0_prefix");
	std::string workspace = GetRequired(tbl, "workspace");

	DecodeT0d1(
		raw_path.c_str(),
		raw_prefix.c_str(),
		workspace.c_str(),
		run,
		true
	);
	return 0;
}
