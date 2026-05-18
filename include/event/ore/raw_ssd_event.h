#pragma once

#include <string>

#include <TTree.h>

namespace forgerib {

struct RawSiliconEvent {
	int energy;
	double time;
	bool cv;
};

void SetupInput(
	TTree *tree,
	RawSiliconEvent &event,
	const std::string &prefix = ""
);

void SetupOutput(TTree *tree, RawSiliconEvent &event);

}
