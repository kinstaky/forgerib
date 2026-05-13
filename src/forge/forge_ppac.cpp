#include "include/forge/forge_ppac.h"

#include <iostream>
#include <cmath>
#include <algorithm>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>

#include "include/event/forge/ppac_event.h"
#include "include/event/raw/raw_ppac_event.h"

namespace glimmer {

constexpr size_t kSlotNum = 1000;

int ForgeWithTrigger(
	const std::vector<double> &trigger_time,
	const char *path,
	const char *output_path,
	const double window,
	bool report = false
) {
	TFile opf(output_path, "recreate");
	TH1F forge_window("fw", "forge window", 200, -window, window);
	TTree opt("tree", "forged ppac");
	PpacEvent ppac;
	SetupOutput(&opt, ppac);

	TFile ipf(path, "read");
	TTree *ipt = (TTree*)ipf.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from "
			<< path << " failed.\n";
		return -1;
	}
	RawPpacEvent raw;
	SetupInput(ipt, raw);

	PpacEvent slots[kSlotNum];
	for (size_t i = 0; i < kSlotNum; ++i) {
		slots[i].flag = 0;
		for (int j = 0; j < 15; ++j) {
			slots[i].valid[j] = false;
		}
	}
	size_t tofill_entry = 0;

	long long total = ipt->GetEntries();
	long long last_percentage = 0;
	if (report) {
		printf("Forging PPAC with trigger   0%%");
		fflush(stdout);
	}
	for (long long entry = 0; entry < total; ++entry) {
		if (report && entry * 100ll / total > last_percentage) {
			last_percentage = entry * 100 / total;
			printf("\b\b\b\b%3lld%%", last_percentage);
			fflush(stdout);
		}
		ipt->GetEntry(entry);
		if (!raw.cv) continue;
		int bit = raw.channel == 4
			? raw.index + 12
			: raw.index*4 + raw.channel;
		// search trigger
		double min_time = 2*window;
		size_t min_time_entry = 0;
		for (
			auto iter = std::lower_bound(
				trigger_time.begin()+tofill_entry,
				trigger_time.end(),
				raw.time - window
			);
			iter != trigger_time.end();
			++iter
		) {
			if (*iter > raw.time + window) break;
			forge_window.Fill(raw.time - *iter);
			if (fabs(*iter - raw.time) < min_time) {
				min_time = fabs(*iter - raw.time);
				min_time_entry = iter - trigger_time.begin();
			}
		}
		if (min_time > window) continue;
		if (min_time_entry < tofill_entry) continue;
		if (min_time_entry - tofill_entry >= kSlotNum) {
			for (size_t fill = tofill_entry; fill <= min_time_entry-kSlotNum; ++fill) {
				ppac = slots[fill%kSlotNum];
				opt.Fill();
				slots[fill%kSlotNum].flag = 0;
				for (size_t i = 0; i < 15; ++i) {
					slots[fill%kSlotNum].valid[i] = 0;
				}
			}
			tofill_entry = min_time_entry - kSlotNum + 1;
		}
		slots[min_time_entry%kSlotNum].flag |= 1 << bit;
		slots[min_time_entry%kSlotNum].valid[bit] = true;
		slots[min_time_entry%kSlotNum].time[bit] = raw.time;
		slots[min_time_entry%kSlotNum].energy[bit] = raw.energy;
	}
	for (size_t fill = tofill_entry; fill < trigger_time.size(); ++fill) {
		ppac = slots[fill%kSlotNum];
		opt.Fill();
	}
	if (report) printf("\b\b\b\b100%%\n");

	opf.cd();
	forge_window.Write();
	opt.Write();
	ipf.Close();
	opf.Close();

	return 0;
}

int ForgeWithoutTrigger(
	const char *path,
	const char *output_path,
	const double window,
	bool report = false
) {
	TFile opf(output_path, "recreate");
	TH1F forge_window("fw", "forge window", 200, -window, window);
	TTree opt("tree", "forged ppac");
	PpacEvent ppac;
	SetupOutput(&opt, ppac);

	TFile ipf(path, "read");
	TTree *ipt = (TTree*)ipf.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from "
			<< path << " failed.\n";
		return -1;
	}
	RawPpacEvent raw;
	SetupInput(ipt, raw);

	ppac.flag = 0;
	double ref_time = -1.0;
	long long total_entries = ipt->GetEntries();
	long long last_percentage = 0;
	if (report) {
		printf("Froging PPAC without trigger   0%%");
		fflush(stdout);
	}
	for (long long entry = 0; entry < total_entries; ++entry) {
		if (report && entry * 100ll / total_entries > last_percentage) {
			last_percentage = entry * 100ll / total_entries;
			printf("\b\b\b\b%3lld%%", last_percentage);
			fflush(stdout);
		}
		ipt->GetEntry(entry);
		if (!raw.cv) continue;
		int bit = raw.channel == 4
			? raw.index + 12
			: raw.index*4 + raw.channel;
		if (ppac.flag == 0) {
			ppac.flag |= 1 << bit;
			ppac.time[bit] = raw.time;
			ppac.energy[bit] = raw.energy;
			ref_time = raw.time;
		} else if (fabs(raw.time - ref_time) < window) {
			forge_window.Fill(raw.time - ref_time);
			if (
				(ppac.flag & (1 << bit)) == 0
				|| raw.energy > ppac.energy[bit]
			) {
				ppac.flag |= 1 << bit;
				ppac.time[bit] = raw.time;
				ppac.energy[bit] = raw.energy;
			}
		} else {
			opt.Fill();
			ppac.flag = 1 << bit;
			ppac.time[bit] = raw.time;
			ppac.energy[bit] = raw.energy;
			ref_time = raw.time;
		}
	}
	opt.Fill();
	if (report) printf("\b\b\b\b100%%\n");
	opf.cd();
	opt.Write();
	ipf.Close();
	opf.Close();

	return 0;
}


int ForgePpac(
	const std::vector<double> &trigger_time,
	const char *path,
	const char *output_path,
	const double window,
	bool report
) {
	if (trigger_time.size() == 0) {
		return ForgeWithoutTrigger(
			path,
			output_path,
			window,
			report
		);
	}
	return ForgeWithTrigger(
		trigger_time,
		path,
		output_path,
		window,
		report
	);
}

}
