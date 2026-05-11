#pragma once

namespace glimmer {

struct RawCsiEvent {
	int index;
	int energy;
	double time;
	bool cv;
};

}