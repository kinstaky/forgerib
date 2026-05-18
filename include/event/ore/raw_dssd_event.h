#pragma once

#include <string>

#include <TTree.h>

namespace forgerib {

constexpr int kDssdSideFront = 0;
constexpr int kDssdSideBack = 1;

struct RawDssdEvent {
	int side;
	int strip;
	int energy;
	double time;
	bool cv;
};

void SetupInput(TTree *tree, RawDssdEvent &event, const std::string &prefix = "");

void SetupOutput(TTree *tree, RawDssdEvent &event);

}
