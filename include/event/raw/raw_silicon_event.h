#ifndef __RAW_SILICON_EVENT_H__
#define __RAW_SILICON_EVENT_H__

namespace glimmer {

struct RawSiliconEvent {
	int side;
	int strip;
	int energy;
	double time;
	bool cfd_valid;
};

}

#endif