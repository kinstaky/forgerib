#ifndef __FUSE_T0CSI_H__
#define __FUSE_T0CSI_H__

#include "include/event/forge/align_event.h"

namespace glimmer {

int FuseT0CsIWithXiaTrigger(
	const std::vector<AlignEvent> &align_events,
	const char *xia_csi_path,
	const char *vme_csi_path,
	const char *output_path,
	bool report = false
);


int FuseT0CsIWithVmeTrigger(
	const std::vector<AlignEvent> &align_events,
	const char *xia_csi_path,
	const char *vme_csi_path,
	const char *output_path,
	bool report = false
);

}

#endif