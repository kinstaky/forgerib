#ifndef __FORGE_T0D2_H__
#define __FORGE_T0D2_H__

#include <vector>

namespace glimmer {

int ForgeT0d2(
	const std::vector<double> &trigger_time,
	const char *path,
	const char *output_path,
	const double window = 1000,
	bool report = false
);

}

#endif
