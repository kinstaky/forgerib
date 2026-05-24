#include "include/crush/crusher.h"

#include <iostream>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "external/cxxopts.hpp"
#include "include/config.h"
#include "include/util.h"
#include "include/event/ore/raw_ppac_event.h"

namespace {

int CrushPpac(
	const char *raw_path,
	const char *prefix,
	const char *output_path,
	int run,
	bool report
) {
	TString filename = TString::Format("%s/ppac_%04d.root", output_path, run);
	TFile opf(filename, "recreate");
	TTree opt("tree", "ppac");
	forgerib::RawPpacEvent ppac;
	forgerib::SetupOutput(&opt, ppac);

	if (report) {
		printf("Crushing PPAC   0%%");
		fflush(stdout);
	}
	std::vector<int> module{0};
	std::vector<int> rate{500};
	forgerib::Crusher crusher(1, run, 2, module, rate, raw_path, prefix);
	for (
		forgerib::DecodeEvent *event = crusher.GetEvent(report);
		event;
		event = crusher.GetEvent(report)
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
	return 0;
}

} // namespace

int main(int argc, char **argv) {
	cxxopts::Options options("crush_ppac", "crush PPAC detector ROOT output");
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
	return CrushPpac(
		config.paths.raw_xia2.c_str(),
		config.prefix.raw_xia2.c_str(),
		output_dir.c_str(),
		run,
		true
	);
}
