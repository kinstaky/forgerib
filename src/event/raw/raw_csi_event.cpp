#include "include/event/raw/raw_csi_event.h"

namespace glimmer {

void SetupInput(TTree *tree, RawCsiEvent &event, const std::string &prefix) {
	tree->SetBranchAddress((prefix+"index").c_str(), &event.index);
	tree->SetBranchAddress((prefix+"energy").c_str(), &event.energy);
	tree->SetBranchAddress((prefix+"time").c_str(), &event.time);
	tree->SetBranchAddress((prefix+"cv").c_str(), &event.cv);
}

void SetupOutput(TTree *tree, RawCsiEvent &event) {
	tree->Branch("index", &event.index, "i/I");
	tree->Branch("energy", &event.energy, "e/I");
	tree->Branch("time", &event.time, "t/D");
	tree->Branch("cv", &event.cv, "cv/O");
}

}
