#ifndef __FORGE_TRIGGER_H__
#define __FORGE_TRIGGER_H__

#include <vector>

namespace forgerib {

int SmeltTrigger(
	const std::vector<double> &trigger_time,
	const char *path,
	const char *output_path,
	const double window = 1000,
	bool report = false
);

}

#endif
