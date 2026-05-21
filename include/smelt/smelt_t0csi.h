#ifndef __FUSE_T0CSI_H__
#define __FUSE_T0CSI_H__

#include "include/event/smelt/align_event.h"

namespace forgerib {

int SmeltT0Csi(
	const std::vector<long long> &vme_entries,
	const char *xia_csi_path,
	const char *vme_csi_path,
	const char *output_path,
	bool report = false
);

}

#endif