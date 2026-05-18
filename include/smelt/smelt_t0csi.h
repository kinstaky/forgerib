#ifndef __FUSE_T0CSI_H__
#define __FUSE_T0CSI_H__

#include "include/event/smelt/align_event.h"

namespace forgerib {

int SmeltT0CsiWithXiaTrigger(
	const std::vector<AlignEvent> &align_events,
	const char *xia_csi_path,
	const char *vme_csi_path,
	const char *output_path,
	bool report = false
);


int SmeltT0CsiWithVmeTrigger(
	const std::vector<AlignEvent> &align_events,
	const char *xia_csi_path,
	const char *vme_csi_path,
	const char *output_path,
	bool report = false
);

}

#endif