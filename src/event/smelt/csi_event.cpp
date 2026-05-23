#include "include/event/smelt/csi_event.h"

namespace forgerib {


// only energy
void SetupInput(TTree *tree, CsiEvent &event, const std::string &prefix) {
	tree->SetBranchAddress((prefix+"flag").c_str(), &event.flag);
	tree->SetBranchAddress((prefix+"valid").c_str(), event.valid);
	tree->SetBranchAddress((prefix+"time").c_str(), event.time);
	tree->SetBranchAddress((prefix+"energy").c_str(), event.energy);
}

void SetupOutput(TTree *tree, CsiEvent &event) {
	tree->Branch("flag", &event.flag, "flag/l");
	tree->Branch("valid", event.valid, "v[36]/O");
	tree->Branch("time", event.time, "t[36]/D");
	tree->Branch("energy", event.energy, "e[36]/I");
}

void Reset(CsiEvent &event) {
	event.flag = 0;
	for (int i = 0; i < 36; ++i) event.valid[i] = false;
}

void Update(
	CsiEvent &event,
	const int index,
	const int energy,
	const double time
) {
	event.flag |= 1ll << index;
	event.valid[index] = true;
	event.energy[index] = energy;
	event.time[index] = time;
}

// with trace

void SetupInput_trace(TTree *tree, Csi_trace_Event &event, const std::string &prefix) {
	// tree->SetBranchAddress((prefix+"valid").c_str(), event.valid);
	tree->SetBranchAddress((prefix+"time").c_str(), event.time);
	tree->SetBranchAddress((prefix+"energy").c_str(), event.energy);
	tree->SetBranchAddress((prefix+"samples").c_str(), event.samples);
	tree->SetBranchAddress((prefix+"hit_num").c_str(), &event.hit_num);
	tree->SetBranchAddress((prefix+"index").c_str(), event.index);
}

void SetupOutput_trace(TTree *tree, Csi_trace_Event &event) {
	tree->Branch("hit_num", &event.hit_num, "hit_num/I");
	// tree->Branch("valid", event.valid, "v[hit_num]/O");
	tree->Branch("time", event.time, "t[hit_num]/D");
	tree->Branch("energy", event.energy, "e[hit_num]/I");
	tree->Branch("samples", event.samples, "samples[hit_num][1000]/s");
	tree->Branch("index", event.index, "index[hit_num]/I");
}

void Reset_trace(Csi_trace_Event &event) {
	event.hit_num = 0;
	}
}

// void Update_trace(
// 	Csi_trace_Event &event,
// 	const int index,
// 	const int energy,
// 	const double time,
// 	const int length,
// 	const  u_int16_t *samples,
// 	const bool out_of_range
// ) {
// 	event.flag |= 1ll << index;
// 	event.valid[index] = true;
// 	event.energy[index] = energy;
// 	event.time[index] = time;
// 	event.length[index] = length;
// 	for (int i=0; i<length; i++) {event.samples[index][i] = samples[i];}
// 	event.out_of_range[index] = out_of_range;



// }



// }
