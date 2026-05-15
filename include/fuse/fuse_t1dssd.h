#ifndef __FUSE_T1DSSD_H__
#define __FUSE_T1DSSD_H__

#include "include/event/forge/align_event.h"

namespace glimmer {

int FuseT1DssdWithXiaTrigger(
	const std::vector<AlignEvent> &align_events,
	const long long xia_entries,
	const char *vme_path,
	const char *output_path,
	bool report = false
);


int FuseT1DssdWithVmeTrigger(
	const std::vector<AlignEvent> &align_events,
	const char *vme_path,
	const char *output_path,
	bool report = false
);

}

#endif