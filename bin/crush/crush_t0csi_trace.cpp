#include <iostream>
#include <memory>

#include <TString.h>
#include <TFile.h>
#include <TTree.h>

#include "external/cxxopts.hpp"
#include "external/toml.hpp"
#include "include/event/ore/raw_csi_event.h"
#include "include/event/ore/decode_event.h"
#include "include/crush/raw_reader.h"
#include "include/config.h"
#include "include/util.h"

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

void DecodeT0CsIWithTrace(
	const char *raw_path,
	const char *raw_prefix,
	const char *output_path,
	int run,
	bool report
) {

	TString filename = TString::Format(
		"%s/t0csi_trace_%04d.root",
		output_path,
		run
	);
	TFile opf(filename, "recreate");
	TTree opt("tree", "decode");
	forgerib::RawCsi_trace_Event csi;
	forgerib::SetupOutput_trace(&opt, csi);

	if (report) {
		printf("Decoding T0 CsI with traces   0%%");
		fflush(stdout);
	}
	size_t last_percentage = 0;


	std::unique_ptr<forgerib::RawReader> readers[2] = {
		std::make_unique<forgerib::RawReader>(raw_path, raw_prefix, run, 2, 5, 100),
		std::make_unique<forgerib::RawReader>(raw_path, raw_prefix, run, 2, 6, 100)
	};
	forgerib::DecodeEvent event[2];   // include energy, time, index
	event[0].used = event[1].used = true;
	forgerib::DecodeTraces traces[2];  // only include trace samples


	size_t total_bytes = readers[0]->FileSize() + readers[1]->FileSize();
	size_t bytes = 0;
	bool finish[2] = {false, false};
	while (bytes < total_bytes) {
		if (report && bytes*100lu/total_bytes > last_percentage) {
			last_percentage = bytes * 100 / total_bytes;
			printf("\b\b\b\b%3lu%%", last_percentage);
			fflush(stdout);
		}

		for (int i = 0; i < 2; ++i) {
			if (finish[i]) continue;
			if (!event[i].used) continue;
			int b = readers[i]->Read(event+i, nullptr, nullptr, nullptr, traces+i); //read raw event
			if (b >= 16) bytes += b;
			else if (b == 0) finish[i] = true;
		}
		if (finish[0] && finish[1]) break;

		if (finish[0]) {
			event[1].used = true;
			if (event[1].channel > 5) continue;
			csi.index = 19 - event[1].channel;
			if (event[1].channel == 5) csi.index = 29;
			csi.energy = event[1].energy;
			csi.time = event[1].time;
			csi.cv = event[1].cfd_valid;
			csi.length = traces[1].length;
			for (int i=0; i< csi.length; i++) {csi.samples[i] = traces[1].samples[i];}
			csi.out_of_range = traces[1].out_of_range;
		} else if (finish[1]) {
			event[0].used = true;
			if (event[0].channel == 6) continue;
			csi.index = 35 - event[0].channel;
			csi.energy = event[0].energy;
			csi.time = event[0].time;
			csi.cv = event[0].cfd_valid;
			csi.length = traces[0].length;
			for (int i=0; i< csi.length; i++) {csi.samples[i] = traces[0].samples[i];}
			csi.out_of_range = traces[0].out_of_range;
		} else {
			if (event[0].timestamp <= event[1].timestamp) {
				event[0].used = true;
				if (event[0].channel == 6) continue;
				csi.index = 35 - event[0].channel;
				csi.energy = event[0].energy;
				csi.time = event[0].time;
				csi.cv = event[0].cfd_valid;
				csi.length = traces[0].length;
				for (int i=0; i< csi.length; i++) {csi.samples[i] = traces[0].samples[i];}
				csi.out_of_range = traces[0].out_of_range;
			} else {
				event[1].used = true;
				if (event[1].channel > 5) continue;
				csi.index = 19 - event[1].channel;
				if (event[1].channel == 5) csi.index = 29;
				csi.energy = event[1].energy;
				csi.time = event[1].time;
				csi.cv = event[1].cfd_valid;
				csi.length = traces[1].length;
				for (int i=0; i< csi.length; i++) {csi.samples[i] = traces[1].samples[i];}
				csi.out_of_range = traces[1].out_of_range;
			}
		}
		opt.Fill();
	}
	if (report) printf("\b\b\b\b100%%\n");

	opf.cd();
	opt.Write();
	opf.Close();
}

int main(int argc, char **argv) {
	cxxopts::Options options("crush_t0csi_trace", "crush T0 CsI traces into ROOT output");
	options.add_options()
		("h,help", "Print usage")
		(
			"c,config",
			"Config file path.",
			cxxopts::value<std::string>()->default_value("config.toml"),
			"path"
		)
		(
			"run",
			"Run number.",
			cxxopts::value<int>(),
			"run"
		);
	options.parse_positional({"run"});
	auto parse_result = options.parse(argc, argv);
	if (parse_result.count("help") || !parse_result.count("run")) {
		std::cout << options.help() << "\n";
		return 0;
	}
	const int run = parse_result["run"].as<int>();
	const std::string config_path = parse_result["config"].as<std::string>();

	forgerib::AppConfig config;
	if (forgerib::LoadConfig(config_path, config)) {
		return -1;
	}
	const std::string output_dir = forgerib::JoinPath(
		config.workspace,
		config.paths.grit
	);

	DecodeT0CsIWithTrace(
		config.paths.raw_xia2.c_str(),
		config.prefix.raw_xia2.c_str(),
		output_dir.c_str(),
		run,
		true
	);
	return 0;
}
