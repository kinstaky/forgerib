#pragma once

#include <string>

#include <TTree.h>

namespace glimmer {

struct RawCsiEvent {
	int index;
	int energy;
	double time;
	bool cv;
};

void SetupInput(TTree *tree, RawCsiEvent &event, const std::string &prefix = "");

void SetupOutput(TTree *tree, RawCsiEvent &event);

}
