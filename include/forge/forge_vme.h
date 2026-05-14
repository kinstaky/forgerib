#ifndef __FORGE_VME_H__
#define __FORGE_VME_H__

#include <vector>
#include <iostream>

#include <TFile.h>
#include <TTree.h>

namespace glimmer {

int ForgeVmeWithTrigger(
	const std::vector<long long> &trigger_times,
	const std::vector<long long> &trigger_entries,
	const char *path,
	const char *output_path,
	const double window,
	const long long range,
	const bool report = false
);

};

#endif	// __FORGE_VME_H__