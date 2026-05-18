#include "include/smelt/smelt_ppac.h"

#include <iostream>
#include <cmath>
#include <algorithm>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>

#include "include/event/smelt/ppac_event.h"
#include "include/event/ore/raw_ppac_event.h"

namespace forgerib {

constexpr size_t kSlotNum = 1000;

void ResetPpacEvent(PpacEvent &ppac) {
	ppac.flag = 0;
	for (int i = 0; i < 15; ++i) {
		ppac.valid[i] = false;
		ppac.time[i] = 0.0;
		ppac.energy[i] = 0;
	}
}

void SetPpacEvent(const RawPpacEvent &raw, PpacEvent &ppac) {
	int bit = raw.channel == 4
		? raw.index + 12
		: raw.index*4 + raw.channel;
	ppac.flag |= 1 << bit;
	ppac.valid[bit] = true;
	ppac.time[bit] = raw.time;
	ppac.energy[bit] = raw.energy;
}

int SmeltWithTrigger(
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
		ResetPpacEvent(slots[i]);
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
				ResetPpacEvent(slots[fill%kSlotNum]);
			}
			tofill_entry = min_time_entry - kSlotNum + 1;
		}
		SetPpacEvent(raw, slots[min_time_entry%kSlotNum]);
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

int SmeltWithoutTrigger(
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

	ResetPpacEvent(ppac);
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
		if (ppac.flag == 0) {
			SetPpacEvent(raw, ppac);
			ref_time = raw.time;
		} else if (fabs(raw.time - ref_time) < window) {
			forge_window.Fill(raw.time - ref_time);
			int bit = raw.channel == 4
				? raw.index + 12
				: raw.index*4 + raw.channel;
			if (
				(ppac.flag & (1 << bit)) == 0
				|| raw.energy > ppac.energy[bit]
			) {
				SetPpacEvent(raw, ppac);
			}
		} else {
			opt.Fill();
			ResetPpacEvent(ppac);
			SetPpacEvent(raw, ppac);
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


int SmeltPpac(
	const std::vector<double> &trigger_time,
	const char *path,
	const char *output_path,
	const double window,
	bool report
) {
	if (trigger_time.size() == 0) {
		return SmeltWithoutTrigger(
			path,
			output_path,
			window,
			report
		);
	}
	return SmeltWithTrigger(
		trigger_time,
		path,
		output_path,
		window,
		report
	);
}

}