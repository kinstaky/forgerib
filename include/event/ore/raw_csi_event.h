#pragma once

#include <string>

#include <TTree.h>

namespace forgerib {

struct RawCsiEvent {
	int index;
	int energy;
	double time;
	bool cv;
};

struct RawCsiTraceEvent {
	int index;
	int energy;
	double time;
	bool cv;
	int length;
	uint16_t samples[5000];
	bool out_of_range;
};


void SetupInput(TTree *tree, RawCsiEvent &event, const std::string &prefix = "");

void SetupOutput(TTree *tree, RawCsiEvent &event);

void SetupInput(TTree *tree, RawCsiTraceEvent &event, const std::string &prefix = "");

void SetupOutput(TTree *tree, RawCsiTraceEvent &event);

}
