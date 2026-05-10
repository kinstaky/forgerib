#include <iostream>
#include <fstream>

#include <TString.h>

#include "include/forge/forge_ppac.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " [run].\n";
		return -1;
	}
	int run = atoi(argv[1]);

	std::ifstream config("config/config.txt");
	std::string raw_path, raw_prefix, workspace;
	config >> raw_path >> raw_prefix >> workspace;

	std::vector<double> tmp;
	TString input_path = TString::Format(
		"%s/decode/ppac_%04d.root",
		workspace,
		run
	);
	TString output_path = TString::Format(
		"%s/forge/ppac_nt_%04d.root",
		workspace,
		run
	);
	return glimmer::ForgePpac(
		tmp,
		input_path.Data(),
		output_path.Data(),
		1000.0,
		true
	);
}