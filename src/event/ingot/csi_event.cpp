#include "include/event/ingot/csi_event.h"

namespace forgerib {

void SetupInput(TTree *tree, CsiTraceEvent &event, const std::string &prefix) {
	tree->SetBranchAddress((prefix+"num").c_str(), &event.num);
	tree->SetBranchAddress((prefix+"time").c_str(), event.time);
	tree->SetBranchAddress((prefix+"energy").c_str(), event.energy);
	tree->SetBranchAddress((prefix+"samples").c_str(), event.samples);
	tree->SetBranchAddress((prefix+"index").c_str(), event.index);
}

void SetupOutput(TTree *tree, CsiTraceEvent &event) {
	tree->Branch("num", &event.num, "num/I");
	tree->Branch("time", event.time, "t[num]/D");
	tree->Branch("energy", event.energy, "e[num]/I");
	tree->Branch("samples", event.samples, "samples[num][1000]/s");
	tree->Branch("index", event.index, "index[num]/I");
}

void Reset(CsiTraceEvent &event) {
	event.num = 0;
}


} // namespace forgerib