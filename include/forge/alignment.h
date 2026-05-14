#ifndef __ALIGNMENT_H__
#define __ALIGNMENT_H__

#include <vector>
#include <map>

#include <TGraph.h>

namespace glimmer {

int Align(
	const std::vector<long long> &xia_times,
	const std::vector<long long> &xia_entries,
	const std::vector<long long> &vme_times,
	const std::vector<long long> &vme_entries,
	const char *output_path,
	int group_num,
	double search_window,
	double search_low_bound,
	double search_high_bound,
	bool report = false,
	bool verbose = false
);

}


#endif			// __ALIGNMENT_H__