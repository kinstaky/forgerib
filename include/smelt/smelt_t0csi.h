#ifndef __FUSE_T0CSI_H__
#define __FUSE_T0CSI_H__

#include "include/event/ingot/align_event.h"

namespace forgerib {

int SmeltT0Csi(
	const std::vector<long long> &vme_entries,
	const char *xia_csi_path,
	const char *vme_csi_path,
	const char *output_path,
	bool report = false
);

int SmeltT0Csi_trace(
	const std::vector<double> &trigger_time,
	const char *path,
	const char *output_path,
	const double window = 1000,
	bool report = false
);

}

#endif
