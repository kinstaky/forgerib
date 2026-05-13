#include "include/decode/decoder.h"

#include <iostream>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "external/toml.hpp"
#include "include/event/raw/raw_csi_event.h"
#include "include/event/raw/raw_ssd_event.h"

namespace {

std::string GetRequired(const toml::table &tbl, const char *key) {
	if (!tbl.contains(key) || !tbl[key].is_string()) {
		std::cerr << "Error: Missing required parameter: " << key << "\n";
		exit(-1);
	}
	return tbl[key].as_string()->get();
}

constexpr int kOthersDecoderNum = 2;
constexpr int kOthersModules[kOthersDecoderNum] = {5, 6};
constexpr int kOthersRates[kOthersDecoderNum] = {100, 100};

bool MapCsiEvent(const glimmer::DecodeEvent &decode, glimmer::RawCsiEvent &csi) {
	// Placeholder mapping: module 5 is routed to CsI and channel is used as index.
	if (decode.module == 5) {
		if (decode.channel == 6) return false;
		csi.index = 35 - decode.channel;
	} else if (decode.module == 6) {
		if (decode.channel > 5) return false;
		csi.index = 19 - decode.channel;
		if (decode.channel == 5) csi.index = 29;
	}
	csi.energy = decode.energy;
	csi.time = decode.time;
	csi.cv = decode.cfd_valid;
	return true;
}

bool MapT0sEvent(
	const glimmer::DecodeEvent &decode,
	glimmer::RawSiliconEvent &silicon
) {
	if (decode.module != 6) return false;
	if (decode.channel != 8) return false;
	silicon.energy = decode.energy;
	silicon.time = decode.time;
	silicon.cv = decode.cfd_valid;
	return true;
}

bool MapT1suEvent(
	const glimmer::DecodeEvent &decode,
	glimmer::RawSiliconEvent &silicon
) {
	if (decode.module != 6) return false;
	if (decode.channel != 10) return false;
	silicon.energy = decode.energy;
	silicon.time = decode.time;
	silicon.cv = decode.cfd_valid;
	return true;
}

bool MapT1sdEvent(
	const glimmer::DecodeEvent &decode,
	glimmer::RawSiliconEvent &silicon
) {
	if (decode.module != 6) return false;
	if (decode.channel != 9) return false;
	silicon.energy = decode.energy;
	silicon.time = decode.time;
	silicon.cv = decode.cfd_valid;
	return true;
}

} // namespace

void DecodeOthers(
	const char *raw_path,
	const char *raw_prefix,
	const char *workspace,
	int run,
	bool report
) {
	TString csi_filename = TString::Format("%s/decode/csi_%04d.root", workspace, run);
	TString t0s_filename = TString::Format("%s/decode/t0s_%04d.root", workspace, run);
	TString t1su_filename = TString::Format("%s/decode/t1su_%04d.root", workspace, run);
	TString t1sd_filename = TString::Format("%s/decode/t1sd_%04d.root", workspace, run);

	TFile csi_file(csi_filename, "recreate");
	TTree csi_tree("tree", "decode");
	glimmer::RawCsiEvent csi;
	glimmer::SetupOutput(&csi_tree, csi);

	TFile t0s_file(t0s_filename, "recreate");
	TTree t0s_tree("tree", "decode");
	glimmer::RawSiliconEvent t0s;
	glimmer::SetupOutput(&t0s_tree, t0s);

	TFile t1su_file(t1su_filename, "recreate");
	TTree t1su_tree("tree", "decode");
	glimmer::RawSiliconEvent t1su;
	glimmer::SetupOutput(&t1su_tree, t1su);

	TFile t1sd_file(t1sd_filename, "recreate");
	TTree t1sd_tree("tree", "decode");
	glimmer::RawSiliconEvent t1sd;
	glimmer::SetupOutput(&t1sd_tree, t1sd);

	if (report) {
		printf("Decoding other   0%%");
		fflush(stdout);
	}
	std::vector<int> module(kOthersModules, kOthersModules + kOthersDecoderNum);
	std::vector<int> rate(kOthersRates, kOthersRates + kOthersDecoderNum);
	glimmer::Decoder decoder(kOthersDecoderNum, run, module, rate, raw_path, raw_prefix);
	for (
		glimmer::DecodeEvent *event = decoder.GetEvent(report);
		event;
		event = decoder.GetEvent(report)
	) {
		if (MapCsiEvent(*event, csi)) {
			csi_tree.Fill();
		} else if (MapT0sEvent(*event, t0s)) {
			t0s_tree.Fill();
		} else if (MapT1suEvent(*event, t1su)) {
			t1su_tree.Fill();
		} else if (MapT1sdEvent(*event, t1sd)) {
			t1sd_tree.Fill();
		}
	}
	if (report) printf("\b\b\b\b100%%\n");

	csi_file.cd();
	csi_tree.Write();
	csi_file.Close();

	t0s_file.cd();
	t0s_tree.Write();
	t0s_file.Close();

	t1su_file.cd();
	t1su_tree.Write();
	t1su_file.Close();

	t1sd_file.cd();
	t1sd_tree.Write();
	t1sd_file.Close();
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

	DecodeOthers(
		raw_path.c_str(),
		raw_prefix.c_str(),
		workspace.c_str(),
		run,
		true
	);
	return 0;
}
