#include "include/event/forge/tafd_event.h"

namespace glimmer {

void SetupInput(TTree *tree, TafdEvent &event, const std::string &prefix) {
	tree->SetBranchAddress((prefix+"flag").c_str(), &event.flag);
	tree->SetBranchAddress((prefix+"valid").c_str(), event.valid);
	tree->SetBranchAddress((prefix+"front_energy").c_str(), event.front_energy);
	tree->SetBranchAddress((prefix+"back_energy").c_str(), event.back_energy);
	tree->SetBranchAddress((prefix+"front_strip").c_str(), event.front_strip);
	tree->SetBranchAddress((prefix+"back_strip").c_str(), event.back_strip);
	tree->SetBranchAddress((prefix+"front_time").c_str(), event.front_time);
}

void SetupOutput(TTree *tree, TafdEvent &event) {
	tree->Branch("flag", &event.flag, "flag/I");
	tree->Branch("valid", event.valid, "valid[6]/O");
	tree->Branch("front_energy", event.front_energy, "front_energy[6]/D");
	tree->Branch("back_energy", event.back_energy, "back_energy[6]/D");
	tree->Branch("front_strip", event.front_strip, "front_strip[6]/I");
	tree->Branch("back_strip", event.back_strip, "back_strip[6]/I");
	tree->Branch("front_time", event.front_time, "front_time[6]/D");
}

}
