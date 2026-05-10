#include "include/decode/decoder.h"

#include <iostream>
#include <fstream>

#include <TString.h>
#include <TFile.h>
#include <TTree.h>

#include "external/toml.hpp"
#include "include/event/raw/raw_ppac_event.h"

std::string GetRequired(
	const toml::table &tbl,
	const char *key
) {
	if (!tbl.contains(key) || !tbl[key].is_string()) {
		std::cerr << "Error: Missing required parameter: "
			<< key << "\n";
		exit(-1);
	}
	return tbl[key].as_string()->get();
}

void DecodePpac(
	const char *raw_path,
	const char *prefix,
	const char *output_path,
	int run,
	bool report
) {
	TString filename = TString::Format(
		"%s/ppac_%04d.root",
		output_path,
		run
	);
	TFile opf(filename, "recreate");
	TTree opt("tree", "ppac");
	glimmer::RawPpacEvent ppac;
	glimmer::SetupOutput(&opt, ppac);

	if (report) {
		printf("Decoding PPAC   0%%");
		fflush(stdout);
	}
	std::vector<int> module{0};
	std::vector<int> rate{500};
	glimmer::Decoder decoder(1, run, module, rate, raw_path, prefix);
	for (
		glimmer::DecodeEvent *event = decoder.GetEvent(report);
		event;
		event = decoder.GetEvent(report)
	) {
		ppac.index = event->channel < 12
			? event->channel / 4
			: event->channel - 12;
		ppac.channel = event->channel < 12
			? event->channel % 4
			: 4;
		ppac.time = event->time;
		ppac.energy = event->energy;
		ppac.cv = event->cfd_valid;
		opt.Fill();
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

	DecodePpac(
		raw_path.c_str(),
		raw_prefix.c_str(),
		(workspace+"/decode").c_str(),
		run,
		true
	);
	return 0;
}