#ifndef __FORGE_CSI_H__
#define __FORGE_CSI_H__

#include <vector>

namespace glimmer {

int ForgeCsi(
	const std::vector<double> &trigger_time,
	const char *path,
	const char *output_path,
	const double window = 1000,
	bool report = false
);

}

#endif
