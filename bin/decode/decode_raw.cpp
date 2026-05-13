#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#include <TString.h>
#include <TFile.h>
#include <TTree.h>

#include "include/decode/raw_reader.h"

constexpr int num = 4;
constexpr int crate = 0;
constexpr int module[num] = {2, 3, 4, 5};
constexpr int rate[num] = {100, 100, 100, 100};
// constexpr int num = 1;
// constexpr int module[num] = {2};
// constexpr int rate[num] = {100};
const char* const raw_path = "/data/test-t0c/raw";
const char* const prefix = "data";


int main(int argc, char **argv) {
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " run\n";
		return -1;
	}

	// run number
	const int run = atoi(argv[1]);

	glimmer::DecodeEvent events[num];
	std::unique_ptr<glimmer::RawReader> readers[num];
	size_t read_size[num];
	for (int i = 0; i < num; ++i) {
		readers[i] = std::make_unique<glimmer::RawReader>(
			raw_path, prefix, run, crate, module[i], rate[i]
		);
		events[i].used = true;
		read_size[i] = 0;
	}

	TString filename = TString::Format(
		"/data/test-t0c/pp_decode/run_%04d.root",
		run
	);
	TFile opf(filename, "recreate");
	TTree opt("tree", "decoded events");
	int mod, channel, energy;
	double time, cfd;
	int64_t timestamp;
	bool cfd_valid;
	opt.Branch("module", &mod, "m/I");
	opt.Branch("channel", &channel, "ch/I");
	opt.Branch("energy", &energy, "e/I");
	opt.Branch("time", &time, "t/D");
	opt.Branch("cfd", &cfd, "cfd/D");
	opt.Branch("cfd_valid", &cfd_valid, "cv/O");
	opt.Branch("timestamp", &timestamp, "ts/L");

	size_t total_size[num];
	for (int i = 0; i < num; ++i) {
		total_size[i] = readers[i]->FileSize();
	}

	// binary file name
	bool module_finish[num];
	for (int i = 0; i < num; ++i) module_finish[i] = false;
	int finished_num = 0;
	while (finished_num < num) {
		// read new events
		for (int i = 0; i < num; ++i) {
			// std::cout << "Module " << module[i] << ": used " << events[i].used << ", finished " << module_finish[i] << "\n";
			if (events[i].used && !module_finish[i]) {
				int bytes = readers[i]->Read(events+i);
				// std::cout << "Read module " << module[i] << " with " << bytes << " bytes.\n";
				if (bytes >= 16) {
					read_size[i] += bytes;
				} else {
					// std::cout << "Get " << bytes << " bytes from module " << module[i] << "\n";
					module_finish[i] = true;
					finished_num += 1;
				}
			}
		}
		if (finished_num == num) {
			std::cout << "Exit with all finished." << std::endl;
			break;
		}

		// find most early event
		int64_t min_ts = -1;
		int min_index = -1;
		for (int i = 0; i < num; ++i) {
			if (events[i].used) continue;
			// std::cout << "  Module " << module[i] << ", ts " << events[i].timestamp << "\n";
			if (min_ts == -1 || events[i].timestamp < min_ts) {
				min_ts = events[i].timestamp;
				min_index = i;
			}
		}

		if (min_index == -1) {
			std::cout << "Exit without min index." << std::endl;
			break;
		}

		// fill
		// std::cout << "  Fill module " << module[min_index] << "\n";
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

	for (int i = 0; i < num; ++i) {
		std::cout << "Module " << module[i] << ", " << read_size[i] << " / " << total_size[i] << "\n";
	}

	// close files
	opt.Write();
	opf.Close();

	return 0;
}