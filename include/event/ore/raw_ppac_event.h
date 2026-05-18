#ifndef __RAW_PPAC_EVENT_H__
#define __RAW_PPAC_EVENT_H__

#include <string>
#include <TTree.h>

namespace forgerib {

struct RawPpacEvent {
	// 0-PPAC0, 1-PPAC1, 2-PPAC2
	int index;
	// 0-x1, 1-x2, 2-y1, 3-y2, 4-cathode
	int channel;
	// time
	double time;
	// energy
	int energy;
	// cfd valid
	bool cv;
};

void SetupInput(TTree* tree, RawPpacEvent &event, const std::string &prefix = "");

void SetupOutput(TTree* tree, RawPpacEvent &event);

}

#endif