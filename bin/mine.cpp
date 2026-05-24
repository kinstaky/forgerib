#include <memory>
#include <iostream>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "external/cxxopts.hpp"
#include "include/config.h"
#include "include/crush/raw_reader.h"
#include "include/util.h"

namespace {

constexpr int kMineNum = 4;
constexpr int kMineCrate = 0;
constexpr int kMineModule[kMineNum] = {2, 3, 4, 5};
constexpr int kMineRate[kMineNum] = {100, 100, 100, 100};

} // namespace

int main(int argc, char **argv) {
	cxxopts::Options options("mine", "mine decoded events into ore output");
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

	forgerib::DecodeEvent events[kMineNum];
	std::unique_ptr<forgerib::RawReader> readers[kMineNum];
	size_t read_size[kMineNum];
	for (int i = 0; i < kMineNum; ++i) {
		readers[i] = std::make_unique<forgerib::RawReader>(
			config.paths.raw_xia0.c_str(),
			config.prefix.raw_xia0.c_str(),
			run,
			kMineCrate,
			kMineModule[i],
			kMineRate[i]
		);
		events[i].used = true;
		read_size[i] = 0;
	}

	const std::string output_dir = forgerib::JoinPath(
		config.workspace,
		config.paths.ore
	);
	TString filename = TString::Format(
		"%s/mine_%04d.root",
		output_dir.c_str(),
		run
	);
	TFile opf(filename, "recreate");
	TTree opt("tree", "mined events");
	int mod = 0;
	int channel = 0;
	int energy = 0;
	double time = 0.0;
	double cfd = 0.0;
	int64_t timestamp = 0;
	bool cfd_valid = false;
	opt.Branch("module", &mod, "m/I");
	opt.Branch("channel", &channel, "ch/I");
	opt.Branch("energy", &energy, "e/I");
	opt.Branch("time", &time, "t/D");
	opt.Branch("cfd", &cfd, "cfd/D");
	opt.Branch("cfd_valid", &cfd_valid, "cv/O");
	opt.Branch("timestamp", &timestamp, "ts/L");

	size_t total_size[kMineNum];
	for (int i = 0; i < kMineNum; ++i) {
		total_size[i] = readers[i]->FileSize();
	}

	bool module_finish[kMineNum];
	for (int i = 0; i < kMineNum; ++i) module_finish[i] = false;
	int finished_num = 0;
	while (finished_num < kMineNum) {
		for (int i = 0; i < kMineNum; ++i) {
			if (events[i].used && !module_finish[i]) {
				int bytes = readers[i]->Read(events + i);
				if (bytes >= 16) {
					read_size[i] += bytes;
				} else {
					module_finish[i] = true;
					finished_num += 1;
				}
			}
		}
		if (finished_num == kMineNum) {
			std::cout << "Exit with all finished." << std::endl;
			break;
		}

		int64_t min_ts = -1;
		int min_index = -1;
		for (int i = 0; i < kMineNum; ++i) {
			if (events[i].used) continue;
			if (min_ts == -1 || events[i].timestamp < min_ts) {
				min_ts = events[i].timestamp;
				min_index = i;
			}
		}

		if (min_index == -1) {
			std::cout << "Exit without min index." << std::endl;
			break;
		}

		mod = events[min_index].module;
		channel = events[min_index].channel;
		energy = events[min_index].energy;
		time = events[min_index].time;
		timestamp = events[min_index].timestamp;
		cfd = events[min_index].cfd;
		cfd_valid = events[min_index].cfd_valid;
		events[min_index].used = true;
		opt.Fill();
	}

	for (int i = 0; i < kMineNum; ++i) {
		std::cout << "Module " << kMineModule[i]
			<< ", " << read_size[i] << " / " << total_size[i] << "\n";
	}

	opt.Write();
	opf.Close();

	return 0;
}
