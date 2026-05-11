#include "include/event/raw/raw_beam_event.h"

namespace glimmer {

void SetupInput(TTree *tree, RawBeamEvent &event, const std::string &prefix) {
	tree->SetBranchAddress((prefix+"type").c_str(), &event.type);
	tree->SetBranchAddress((prefix+"energy").c_str(), &event.energy);
	tree->SetBranchAddress((prefix+"time").c_str(), &event.time);
	tree->SetBranchAddress((prefix+"cv").c_str(), &event.cv);
}

void SetupOutput(TTree *tree, RawBeamEvent &event) {
	tree->Branch("type", &event.type, "i/I");
	tree->Branch("energy", &event.energy, "e/I");
	tree->Branch("time", &event.time, "t/D");
	tree->Branch("cv", &event.cv, "cv/O");
}

}
