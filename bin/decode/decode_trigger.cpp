#include <iostream>

#include <TString.h>
#include <TFile.h>
#include <TTree.h>

#include "external/toml.hpp"
#include "include/event/raw/raw_trigger_event.h"
#include "include/decode/decoder.h"

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

void DecodeTrigger(
	const char *raw_path,
	const char *raw_prefix,
	const char *workspace,
	int run,
	bool report
) {
	TString filename = TString::Format(
		"%s/decode/trigger_%04d.root",
		workspace,
		run
	);
	TFile opf(filename, "recreate");
	TTree opt("tree", "decode");
	glimmer::RawTriggerEvent trigger;
	glimmer::SetupOutput(&opt, trigger);

	if (report) {
		printf("Decoding trigger   0%%");
		fflush(stdout);
	}
	std::vector<int> module{0};
	std::vector<int> rate{250};
	glimmer::Decoder decoder(1, run, 0, module, rate, raw_path, raw_prefix);
	for (
		glimmer::DecodeEvent *event = decoder.GetEvent(report);
		event;
		event = decoder.GetEvent(report)
	) {
		if (event->channel >= 4 && event->channel <= 9) {
			trigger.type = event->channel - 4;
			trigger.time = event->time;
			trigger.cv = event->cfd_valid;
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

	DecodeTrigger(
		raw_path.c_str(),
		raw_prefix.c_str(),
		workspace.c_str(),
		run,
		true
	);
	return 0;
}