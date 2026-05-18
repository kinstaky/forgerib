#pragma once

#include <string>

#include <TTree.h>

namespace forgerib {

constexpr int kBeamT1 = 0;
constexpr int kBeamT2 = 1;
constexpr int kBeamSi = 2;

struct RawBeamEvent {
	int type;
	int energy;
	double time;
	bool cv;
};

void SetupInput(TTree *tree, RawBeamEvent &event, const std::string &prefix = "");

void SetupOutput(TTree *tree, RawBeamEvent &event);


}
