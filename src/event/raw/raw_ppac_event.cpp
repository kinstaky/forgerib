#include "include/event/raw/raw_ppac_event.h"

namespace glimmer {

void SetupInput(TTree* tree, RawPpacEvent &event, const std::string &prefix) {
	tree->SetBranchAddress((prefix+"index").c_str(), &event.index);
	tree->SetBranchAddress((prefix+"channel").c_str(), &event.channel);
	tree->SetBranchAddress((prefix+"time").c_str(), &event.time);
	tree->SetBranchAddress((prefix+"energy").c_str(), &event.energy);
	tree->SetBranchAddress((prefix+"cv").c_str(), &event.cv);
}

void SetupOutput(TTree *tree, RawPpacEvent &event) {
	tree->Branch("index", &event.index, "i/I");
	tree->Branch("channel", &event.channel, "ch/I");
	tree->Branch("time", &event.time, "t/D");
	tree->Branch("energy", &event.energy, "e/I");
	tree->Branch("cv", &event.cv, "cv/O");
}

}