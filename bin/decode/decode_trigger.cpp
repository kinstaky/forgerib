#include <iostream>

#include <TString.h>
#include <TFile.h>
#include <TTree.h>

#include "external/toml.hpp"
#include "include/event/raw/raw_trigger_event.h"
#include "include/event/raw/decode_event.h"
#include "include/decode/raw_reader.h"

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
	size_t last_percentage = 0;

	glimmer::RawReader reader(
		raw_path, raw_prefix, run, 0, 0, 250
	);
	glimmer::DecodeEvent event;
	glimmer::RawExternalTime *external_time;

	size_t total_bytes = reader.FileSize();
	size_t bytes = 0;
	while (bytes < total_bytes) {
		if (report && bytes*100lu/total_bytes > last_percentage) {
			last_percentage = bytes * 100 / total_bytes;
			printf("\b\b\b\b%3lu%%", last_percentage);
			fflush(stdout);
		}
		size_t read_bytes = reader.Read(&event, nullptr, nullptr, &external_time);
		if (read_bytes >= 16) bytes += read_bytes;
		else if (read_bytes == 0) break;
		if (event.channel >= 4 && event.channel <= 9) {
			trigger.type = event.channel - 4;
			trigger.time = event.time;
			trigger.external_time =
				((long long)(external_time->high) << 32) | ((long long)(external_time->low));
			trigger.cv = event.cfd_valid;
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