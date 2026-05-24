#include "include/crush/crusher.h"

#include <iostream>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "external/cxxopts.hpp"
#include "include/config.h"
#include "include/util.h"
#include "include/event/ore/raw_csi_event.h"
#include "include/event/ore/raw_ssd_event.h"

namespace {

constexpr int kOthersCrusherNum = 2;
constexpr int kOthersModules[kOthersCrusherNum] = {5, 6};
constexpr int kOthersRates[kOthersCrusherNum] = {100, 100};

bool MapCsiEvent(const forgerib::DecodeEvent &decode, forgerib::RawCsiEvent &csi) {
	if (decode.module == 5) {
		if (decode.channel == 6) return false;
		csi.index = 35 - decode.channel;
	} else if (decode.module == 6) {
		if (decode.channel > 5) return false;
		csi.index = 19 - decode.channel;
		if (decode.channel == 5) csi.index = 29;
	} else {
		return false;
	}
	csi.energy = decode.energy;
	csi.time = decode.time;
	csi.cv = decode.cfd_valid;
	return true;
}

bool MapT0sEvent(
	const forgerib::DecodeEvent &decode,
	forgerib::RawSiliconEvent &silicon
) {
	if (decode.module != 6 || decode.channel != 8) return false;
	silicon.energy = decode.energy;
	silicon.time = decode.time;
	silicon.cv = decode.cfd_valid;
	return true;
}

bool MapT1suEvent(
	const forgerib::DecodeEvent &decode,
	forgerib::RawSiliconEvent &silicon
) {
	if (decode.module != 6 || decode.channel != 10) return false;
	silicon.energy = decode.energy;
	silicon.time = decode.time;
	silicon.cv = decode.cfd_valid;
	return true;
}

bool MapT1sdEvent(
	const forgerib::DecodeEvent &decode,
	forgerib::RawSiliconEvent &silicon
) {
	if (decode.module != 6 || decode.channel != 9) return false;
	silicon.energy = decode.energy;
	silicon.time = decode.time;
	silicon.cv = decode.cfd_valid;
	return true;
}

int CrushOthers(
	const char *raw_path,
	const char *raw_prefix,
	const char *output_path,
	int run,
	bool report
) {
	TString t0csi_filename = TString::Format("%s/t0csi_%04d.root", output_path, run);
	TString t0s_filename = TString::Format("%s/t0s_%04d.root", output_path, run);
	TString t1su_filename = TString::Format("%s/t1su_%04d.root", output_path, run);
	TString t1sd_filename = TString::Format("%s/t1sd_%04d.root", output_path, run);

	TFile t0csi_file(t0csi_filename, "recreate");
	TTree t0csi_tree("tree", "crush");
	forgerib::RawCsiEvent csi;
	forgerib::SetupOutput(&t0csi_tree, csi);

	TFile t0s_file(t0s_filename, "recreate");
	TTree t0s_tree("tree", "crush");
	forgerib::RawSiliconEvent t0s;
	forgerib::SetupOutput(&t0s_tree, t0s);

	TFile t1su_file(t1su_filename, "recreate");
	TTree t1su_tree("tree", "crush");
	forgerib::RawSiliconEvent t1su;
	forgerib::SetupOutput(&t1su_tree, t1su);

	TFile t1sd_file(t1sd_filename, "recreate");
	TTree t1sd_tree("tree", "crush");
	forgerib::RawSiliconEvent t1sd;
	forgerib::SetupOutput(&t1sd_tree, t1sd);

	if (report) {
		printf("Crushing other   0%%");
		fflush(stdout);
	}
	std::vector<int> module(kOthersModules, kOthersModules + kOthersCrusherNum);
	std::vector<int> rate(kOthersRates, kOthersRates + kOthersCrusherNum);
	forgerib::Crusher crusher(kOthersCrusherNum, run, 2, module, rate, raw_path, raw_prefix);
	for (
		forgerib::DecodeEvent *event = crusher.GetEvent(report);
		event;
		event = crusher.GetEvent(report)
	) {
		if (MapCsiEvent(*event, csi)) {
			t0csi_tree.Fill();
		} else if (MapT0sEvent(*event, t0s)) {
			t0s_tree.Fill();
		} else if (MapT1suEvent(*event, t1su)) {
			t1su_tree.Fill();
		} else if (MapT1sdEvent(*event, t1sd)) {
			t1sd_tree.Fill();
		}
	}
	if (report) printf("\b\b\b\b100%%\n");

	t0csi_file.cd();
	t0csi_tree.Write();
	t0csi_file.Close();

	t0s_file.cd();
	t0s_tree.Write();
	t0s_file.Close();

	t1su_file.cd();
	t1su_tree.Write();
	t1su_file.Close();

	t1sd_file.cd();
	t1sd_tree.Write();
	t1sd_file.Close();
	return 0;
}

} // namespace

int main(int argc, char **argv) {
	cxxopts::Options options("crush_others", "crush auxiliary detector ROOT output");
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
	return CrushOthers(
		config.paths.raw_xia2.c_str(),
		config.prefix.raw_xia2.c_str(),
		output_dir.c_str(),
		run,
		true
	);
}
