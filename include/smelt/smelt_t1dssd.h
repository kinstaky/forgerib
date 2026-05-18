#ifndef __FUSE_T1DSSD_H__
#define __FUSE_T1DSSD_H__

#include "include/event/smelt/align_event.h"

namespace forgerib {

int SmeltT1DssdWithXiaTrigger(
	const std::vector<AlignEvent> &align_events,
	const long long xia_entries,
	const char *vme_path,
	const char *output_path,
	bool report = false
);


int SmeltT1DssdWithVmeTrigger(
	const std::vector<AlignEvent> &align_events,
	const char *vme_path,
	const char *output_path,
	bool report = false
);

}

#endif