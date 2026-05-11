#pragma once

namespace glimmer {

constexpr int kDssdSideFront = 0;
constexpr int kDssdSideBack = 1;

struct RawDssdEvent {
	int side;
	int strip;
	int energy;
	double time;
	bool cv;
};

}