#include "include/event/raw/raw_dssd_event.h"

namespace glimmer {

void SetupInput(TTree *tree, RawDssdEvent &event, const std::string &prefix) {
	tree->SetBranchAddress((prefix+"side").c_str(), &event.side);
	tree->SetBranchAddress((prefix+"strip").c_str(), &event.strip);
	tree->SetBranchAddress((prefix+"energy").c_str(), &event.energy);
	tree->SetBranchAddress((prefix+"time").c_str(), &event.time);
	tree->SetBranchAddress((prefix+"cv").c_str(), &event.cv);
}

void SetupOutput(TTree *tree, RawDssdEvent &event) {
	tree->Branch("side", &event.side, "side/I");
	tree->Branch("strip", &event.strip, "strip/I");
	tree->Branch("energy", &event.energy, "e/I");
	tree->Branch("time", &event.time, "t/D");
	tree->Branch("cv", &event.cv, "cv/O");
}

}
