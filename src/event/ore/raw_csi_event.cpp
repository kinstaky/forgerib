#include "include/event/ore/raw_csi_event.h"

namespace forgerib {

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

void SetupInput_trace(TTree *tree, RawCsi_trace_Event &event, const std::string &prefix) {
	tree->SetBranchAddress((prefix+"index").c_str(), &event.index);
	tree->SetBranchAddress((prefix+"energy").c_str(), &event.energy);
	tree->SetBranchAddress((prefix+"time").c_str(), &event.time);
	tree->SetBranchAddress((prefix+"cv").c_str(), &event.cv);
	tree->SetBranchAddress((prefix+"slength").c_str(), &event.length);
	tree->SetBranchAddress((prefix+"samples").c_str(), event.samples);
	tree->SetBranchAddress((prefix+"trace_out_of_range").c_str(), &event.out_of_range);
}


void SetupOutput_trace(TTree *tree, RawCsi_trace_Event &event) {
	tree->Branch("index", &event.index, "i/I");
	tree->Branch("energy", &event.energy, "e/I");
	tree->Branch("time", &event.time, "t/D");
	tree->Branch("cv", &event.cv, "cv/O");
	tree->Branch("slength", &event.length, "sl/I");
	tree->Branch("samples", event.samples, "sa[sl]/s");
	tree->Branch("trace_out_of_range", &event.out_of_range, "toor/O");
}


}
