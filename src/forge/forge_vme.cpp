#include "include/forge/forge_vme.h"

#include <algorithm>

#include <TGraph.h>

#include "include/event/forge/vme_event.h"

namespace glimmer {

int ForgeVmeWithTrigger(
	const std::vector<long long> &trigger_times,
	const std::vector<long long> &trigger_entries,
	const char *path,
	const char *output_path,
	const double window,
	const long long range,
	const bool report
) {
	TFile opf(output_path, "recreate");
	TTree opt("tree", "forge vme tree");
	TGraph g_rate;
	VmeEvent event;
	SetupOutput(&opt, event);

	TFile ipf(path, "read");
	TTree *ipt = (TTree*)ipf.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from "
			<< path << " failed.\n";
		return -1;
	}
	unsigned long long sdc[32];
	ipt->SetBranchAddress("sdc", sdc);

	long long first_xia_ts = trigger_times[0];
	std::vector<long long> relatvie_times;
	for (size_t i = 0; i < trigger_times.size(); ++i) {
		relatvie_times.push_back(trigger_times[i] - first_xia_ts);
	}

	long long sample_start = ipt->GetEntries() > 1000 ? 500 : ipt->GetEntries() / 2;
	long long sample_size = ipt->GetEntries() > 1000 ? 500 : ipt->GetEntries();
	std::vector<long long> samples;
	for (long long entry = sample_start; entry < sample_start+sample_size; ++entry) {
		ipt->GetEntry(entry);
		samples.push_back((long long)sdc[1]);
	}
	long long first_sample = samples[0];
	for (size_t i = 0; i < samples.size(); ++i) {
		samples[i] -= first_sample;
	}

	// search
	double max_rate = 0.0;
	long long average_offset = 0;
	for (long long offset = -range; offset < range; ++offset) {
		int matched = 0;
		long long sum_offsets = 0;
		for (const long long &sample : samples) {
			long long sample_matched = 0;
			long long last_offset = 0;
			for (
				auto iter = std::lower_bound(
					relatvie_times.begin(), relatvie_times.end(), sample+offset*window
				);
				iter != relatvie_times.end();
				++iter
			) {
				// stop
				if (*iter > sample + (offset+1)*window) break;
				++sample_matched;
				last_offset = *iter - sample;
			}
			if (sample_matched == 1) {
				sum_offsets += last_offset;
			}
			if (sample_matched) ++matched;
		}
		double match_rate = double(matched) / samples.size();
		if (match_rate > max_rate) {
			max_rate = match_rate;
			average_offset = sum_offsets / matched;
		}
		g_rate.AddPoint(offset, match_rate);
	}
	if (report) {
		std::cout << "Relative average offset: " << average_offset << "\n";
		std::cout << "Average offset: " << average_offset + first_xia_ts - first_sample << "\n";
	}

	// search in window
	if (report) {
		printf("Aliging   0%%");
		fflush(stdout);
	}
	long long last_scaler = 0;
	long long bitflip_offset = 0;
	long long total = ipt->GetEntries();
	long long last_percentage = 0;
	for (long long entry = 0; entry < total; ++entry) {
		if (report) {
			if (entry * 100ll / total > last_percentage) {
				last_percentage = entry * 100ll / total;
				printf("\b\b\b\b%3lld%%", last_percentage);
				fflush(stdout);
			}
		}
		ipt->GetEntry(entry);
		if (sdc[1] == 0) continue;
		long long scaler = (long long)sdc[1] + bitflip_offset;
		if (scaler < last_scaler) {
			bitflip_offset += 1ll << 32;
			scaler += 1ll << 32;
		}
		last_scaler = scaler;
		event.vme_ts = scaler - first_sample;
		long long diff = window*10;
		long long found_entry = -1;
		for (
			auto iter = std::lower_bound(
				relatvie_times.begin(),
				relatvie_times.end(),
				event.vme_ts+average_offset-window-event.vme_ts/3000000
			);
			iter != relatvie_times.end();
			++iter
		) {
			if (*iter > event.vme_ts+average_offset+window-event.vme_ts/3000000) break;
			// std::cout << "Entry " << entry << ", XIA " << *iter << ", VME " << vme_ts << "\n";
			if (abs(*iter - event.vme_ts) < diff) {
				event.xia_ts = *iter;
				diff = abs(*iter - event.vme_ts);
				found_entry = trigger_entries[iter-relatvie_times.begin()];
			}
		}
		if (found_entry == -1) continue;
		event.vme_ts = scaler;
		event.xia_ts += first_xia_ts;
		event.xia_entry = found_entry;
		opt.Fill();
	}
	if (report) printf("\b\b\b\b100%%\n");

	ipf.Close();

	opf.cd();
	opt.Write();
	g_rate.Write("gr");
	opf.Close();

	return 0;
}

} // namespace name