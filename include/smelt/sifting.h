#ifndef __SIFTING_H__
#define __SIFTING_H__

#include <vector>
#include <map>

#include <TGraph.h>

namespace forgerib {

int Sift(
	const std::vector<double> &xia_times,
	const std::vector<long long> &xia_entries,
	const std::vector<double> &vme_times,
	const std::vector<long long> &vme_entries,
	const char *output_path,
	const double window,
	const size_t range = 1000,
	const size_t vme_window = 5000,
	const size_t xia_window = 10000,
	bool report = false
);

} // namespace forgerib


#endif // __SIFTING_H__
