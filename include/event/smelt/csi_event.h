#ifndef __CSI_EVENT_H__
#define __CSI_EVENT_H__

#include <string>

#include <TTree.h>

namespace forgerib {


// only with energy
struct CsiEvent {
	unsigned long long flag;
	bool valid[36];
	double time[36];
	int energy[36];
};

void SetupInput(TTree *tree, CsiEvent &event, const std::string &prefix = "");

void SetupOutput(TTree *tree, CsiEvent &event);

void Reset(CsiEvent &event);

void Update(CsiEvent &event, const int index, const int energy, const double time);



// with trace
struct Csi_trace_Event {
	// bool valid[21];
	double time[21];
	int energy[21];
	uint16_t samples[21][1000];
	int index[21];
	int hit_num;
};

void SetupInput_trace(TTree *tree, Csi_trace_Event &event, const std::string &prefix = "");

void SetupOutput_trace(TTree *tree, Csi_trace_Event &event);

void Reset_trace(Csi_trace_Event &event);

void Update_trace(Csi_trace_Event &event
					, const int index
					, const int energy
					, const double time
					, const int length
					, const  u_int16_t *samples
					, const bool out_of_range);
}
#endif
