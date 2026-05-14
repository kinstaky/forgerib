#include "include/forge/colossus.h"

#include <algorithm>

#include <TGraph.h>

namespace glimmer {

int ReadVmeTrigger(
	const char *trigger_path,
	std::vector<long long> &timestamps,
	std::vector<long long> &entries
) {
	TFile trig_file(trigger_path, "read");
	TTree *ipt = (TTree*)trig_file.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from "
			<< trigger_path << " failed.\n";
		return -1;
	}
	short sid, ch;
	long long ets;
	ipt->SetBranchAddress("sid", &sid);
	ipt->SetBranchAddress("ch", &ch);
	ipt->SetBranchAddress("ets", &ets);

	timestamps.clear();
	entries.clear();
	for (long long entry = 0; entry < ipt->GetEntriesFast(); ++entry) {
		ipt->GetEntry(entry);
		if (sid != 2 || ch != 5) continue;
		timestamps.push_back(ets);
		entries.push_back(entry);
	}
	trig_file.Close();
	return 0;
}

int ForgeVme(
	const char *trigger_path,
	const char *vme_path,
	const char *output_path,
	const long long range,
	const long long window
) {
	TFile opf(output_path, "recreate");
	TTree opt("tree", "forge vme tree");
	int valid = 0;
	long long xia_ts;
	long long vme_ts;
	long long vme_sdc;
	opt.Branch("valid", &valid, "v/I");
	opt.Branch("xia_ts", &xia_ts, "xts/L");
	opt.Branch("vme_ts", &vme_ts, "vts/L");
	opt.Branch("vme_sdc", &vme_sdc, "vsdc/L");
	TGraph g_rate;

	std::vector<long long> xia_timestamps;
	std::vector<long long> xia_entries;
	if (ReadVmeTrigger(trigger_path, xia_timestamps, xia_entries)) return -1;
	long long first_xia_ts = xia_timestamps[0];
	for (size_t i = 0; i < xia_timestamps.size(); ++i) {
		xia_timestamps[i] -= first_xia_ts;
	}

	TFile vme_file(vme_path, "read");
	TTree *ipt = (TTree*)vme_file.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from "
			<< vme_path << " failed.\n";
		return -2;
	}
	unsigned long long sdc[32];
	ipt->SetBranchAddress("sdc", sdc);

	long long sample_size = ipt->GetEntries() > 100 ? 100 : ipt->GetEntries();
	std::vector<long long> samples;
	for (long long entry = 0; entry < sample_size; ++entry) {
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
					xia_timestamps.begin(), xia_timestamps.end(), sample+offset*window
				);
				iter != xia_timestamps.end();
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
	std::cout << "Relative average offset: " << average_offset << "\n";
	std::cout << "Average offset: " << average_offset + first_xia_ts - first_sample << "\n";

	// search in window
	long long last_fill_entry = -1;
	long long last_scaler = 0;
	long long bitflip_offset = 0;
	for (long long entry = 0; entry < ipt->GetEntriesFast(); ++entry) {
		ipt->GetEntry(entry);
		if (sdc[1] == 0) continue;
		long long scaler = (long long)sdc[1] + bitflip_offset;
		if (scaler < last_scaler) {
			bitflip_offset += 1ll << 32;
			scaler += 1ll << 32;
		}
		last_scaler = scaler;
		vme_ts = scaler - first_sample;
		// xia_ts = xia_timestamps[entry];
		// vme_sdc = (long long)sdc[1];
		// valid = 1;
		// opt.Fill();
		vme_ts = scaler - first_sample;
		long long diff = window*10;
		long long found_entry = -1;
		for (
			auto iter = std::lower_bound(
				xia_timestamps.begin(), xia_timestamps.end(), vme_ts+average_offset-window-vme_ts/3000000
			);
			iter != xia_timestamps.end();
			++iter
		) {
			if (*iter > vme_ts+average_offset+window-vme_ts/3000000) break;
			// std::cout << "Entry " << entry << ", XIA " << *iter << ", VME " << vme_ts << "\n";
			if (abs(*iter - vme_ts) < diff) {
				xia_ts = *iter;
				diff = abs(*iter - vme_ts);
				found_entry = xia_entries[iter-xia_timestamps.begin()];
			}
		}
		if (found_entry == -1) continue;
		while (found_entry > last_fill_entry+1) {
			valid = 0;
			++last_fill_entry;
			opt.Fill();
		}
		valid = 1;
		vme_ts = scaler;
		xia_ts += first_xia_ts;
		++last_fill_entry;
		opt.Fill();
	}
	for (long long entry = last_fill_entry+1; entry <= xia_entries.back(); ++entry) {
		valid = 0;
		opt.Fill();
	}

	vme_file.Close();

	opf.cd();
	opt.Write();
	g_rate.Write("gr");
	opf.Close();

	return 0;
}

} // namespace name