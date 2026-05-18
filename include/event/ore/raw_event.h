#ifndef __RAW_EVENT_H__
#define __RAW_EVENT_H__

namespace forgerib {

struct RawHeader {
	unsigned int data[4];
};

struct RawEnergySum {
	unsigned int trailing;
	unsigned int leading;
	unsigned int gap;
	float baseline;
};

struct RawQDC {
	unsigned int sum[8];
};

struct RawExternalTime {
	unsigned int low;
	unsigned int high;
};

}

#endif // __RAW_EVENT_H__
