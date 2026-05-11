#include "include/decode/decoder.h"

#include <iostream>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "external/toml.hpp"
#include "include/event/raw/raw_beam_event.h"

namespace {

std::string GetRequired(const toml::table &tbl, const char *key) {
	if (!tbl.contains(key) || !tbl[key].is_string()) {
		std::cerr << "Error: Missing required parameter: " << key << "\n";
		exit(-1);
	}
	return tbl[key].as_string()->get();
}

constexpr int kBeamDecoderNum = 1;
constexpr int kBeamModules[kBeamDecoderNum] = {0};
constexpr int kBeamRates[kBeamDecoderNum] = {250};

bool MapBeamEvent(const glimmer::DecodeEvent &decode, glimmer::RawBeamEvent &beam) {
	// Placeholder mapping copied as framework only.
	switch (decode.channel) {
	case 0:
		beam.type = glimmer::kBeamT1;
		break;
	case 1:
		beam.type = glimmer::kBeamT2;
		break;
	case 2:
		beam.type = glimmer::kBeamSi;
		break;
	default:
		return false;
	}
	beam.energy = decode.energy;
	beam.time = decode.time;
	beam.cv = decode.cfd_valid;
	return true;
}

} // namespace

void DecodeBeam(
	const char *raw_path,
	const char *raw_prefix,
	const char *workspace,
	int run,
	bool report
) {
	TString filename = TString::Format("%s/decode/beam_%04d.root", workspace, run);
	TFile opf(filename, "recreate");
	TTree opt("tree", "decode");
	glimmer::RawBeamEvent beam;
	glimmer::SetupOutput(&opt, beam);

	if (report) {
		printf("Decoding beam   0%%");
		fflush(stdout);
	}
	std::vector<int> module(kBeamModules, kBeamModules + kBeamDecoderNum);
	std::vector<int> rate(kBeamRates, kBeamRates + kBeamDecoderNum);
	glimmer::Decoder decoder(kBeamDecoderNum, run, module, rate, raw_path, raw_prefix);
	for (
		glimmer::DecodeEvent *event = decoder.GetEvent(report);
		event;
		event = decoder.GetEvent(report)
	) {
		if (MapBeamEvent(*event, beam)) {
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

	DecodeBeam(
		raw_path.c_str(),
		raw_prefix.c_str(),
		workspace.c_str(),
		run,
		true
	);
	return 0;
}
