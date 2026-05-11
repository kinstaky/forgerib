#include "include/event/forge/dssd_event.h"

namespace glimmer {

void SetupInput(TTree *tree, DssdEvent &event, const std::string &prefix) {
	tree->SetBranchAddress((prefix+"front_num").c_str(), &event.front_num);
	tree->SetBranchAddress((prefix+"front_strip").c_str(), event.front_strip);
	tree->SetBranchAddress((prefix+"front_energy").c_str(), event.front_energy);
	tree->SetBranchAddress((prefix+"front_time").c_str(), event.front_time);
	tree->SetBranchAddress((prefix+"back_num").c_str(), &event.back_num);
	tree->SetBranchAddress((prefix+"back_strip").c_str(), event.back_strip);
	tree->SetBranchAddress((prefix+"back_energy").c_str(), event.back_energy);
	tree->SetBranchAddress((prefix+"back_time").c_str(), event.back_time);
}

void SetupOutput(TTree *tree, DssdEvent &event) {
	tree->Branch("front_num", &event.front_num, "fn/I");
	tree->Branch("front_strip", event.front_strip, "fs[8]/I");
	tree->Branch("front_energy", event.front_energy, "fe[8]/D");
	tree->Branch("front_time", event.front_time, "ft[8]/D");
	tree->Branch("back_num", &event.back_num, "bn/I");
	tree->Branch("back_strip", event.back_strip, "bs[8]/I");
	tree->Branch("back_energy", event.back_energy, "be[8]/D");
	tree->Branch("back_time", event.back_time, "bt[8]/D");
}

}
