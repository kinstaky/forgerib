#pragma once

namespace glimmer {

constexpr int kBeamT1 = 0;
constexpr int kBeamT2 = 1;
constexpr int kBeamSi = 2;

struct RawBeamEvent {
	int type;
	int energy;
	double time;
	bool cv;
};


}