#include <iostream>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>

#include "external/toml.hpp"

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " [run].\n";
		return 0;
	}
	int run = atoi(argv[1]);

	toml::table tbl = toml::parse_file("config.toml");
	std::string workspace = tbl["workspace"].as_string()->get();


	std::vector<long long> times;
	TString input_path = TString::Format(
		"%s/forge/trigger_vme_%04d.root",
		workspace.c_str(),
		run
	);
	TFile input_file(input_path, "read");
	TTree *ipt= (TTree*)input_file.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from " << input_path << " failed.\n";
		return -1;
	}
	bool valid;
	long long time;
	ipt->SetBranchAddress("time", &time);
	ipt->SetBranchAddress("valid", &valid);

	printf("Reading   0%%");
	fflush(stdout);
	long long total = ipt->GetEntries();
	long long last_percentage = 0;
	for (long long entry = 0; entry < total; ++entry) {
		if (entry * 100ll / total > last_percentage) {
			last_percentage = entry * 100ll / total;
			printf("\b\b\b\b%3lld%%", last_percentage);
			fflush(stdout);
		}
		ipt->GetEntry(entry);
		if (!valid) continue;
		times.push_back(time);
	}
	printf("\b\b\b\b100%%\n");
	input_file.Close();


	TString output_filename = TString::Format(
		"%s/tools/search_cycle_%04d.root",
		workspace.c_str(),
		run
	);
	TFile opf(output_filename, "recreate");
	TH1F time_diff("hd", "Time difference", 10000, 0, 100'000'000);

	printf("Looping   0%%");
	fflush(stdout);
	size_t last = 0;
	for (size_t i = 0; i < times.size(); ++i) {
		if (i * 100 / total > last) {
			last = i * 100ll / total;
			printf("\b\b\b\b%3lu%%", last);
			fflush(stdout);
		}
		for (size_t j = i+1; j < times.size(); ++j) {
			if (times[j]-times[i] > 100'000'000) break;
			time_diff.Fill(times[j]-times[i]);
		}
	}
	printf("\b\b\b\b100%%\n");

	opf.cd();
	time_diff.Write();
	opf.Close();
	return 0;
}
