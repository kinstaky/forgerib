#include "include/forge/forge_beam.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include <TFile.h>
#include <TH1F.h>
#include <TTree.h>

#include "include/event/forge/beam_event.h"
#include "include/event/raw/raw_beam_event.h"

namespace glimmer {

namespace {

constexpr size_t kSlotNum = 1000;

void ResetBeamEvent(BeamEvent &beam) {
	beam.flag = 0;
	beam.valid = false;
	beam.tof = 0.0;
}

void UpdateBeamEvent(
	const RawBeamEvent &raw,
	BeamEvent &beam
) {
	if (raw.type < 0 || raw.type > 2) return;
	if ((beam.flag & (1 << raw.type)) == 0) {
		beam.flag |= 1 << raw.type;
		if (raw.type == 0) beam.tof -= raw.time;
		else if (raw.type == 1) beam.tof += raw.time;
		if ((beam.flag & 0x3) == 0x3) beam.valid = true;
	}
}

int ForgeWithTrigger(
	const std::vector<double> &trigger_time,
	const char *path,
	const char *output_path,
	const double window,
	bool report
) {
	TFile opf(output_path, "recreate");
	TH1F forge_window("fw", "forge window", 200, -window, window);
	TTree opt("tree", "forged beam");
	BeamEvent beam;
	SetupOutput(&opt, beam);

	TFile ipf(path, "read");
	TTree *ipt = (TTree*)ipf.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from " << path << " failed.\n";
		return -1;
	}
	RawBeamEvent raw;
	SetupInput(ipt, raw);

	BeamEvent slots[kSlotNum];
	for (size_t i = 0; i < kSlotNum; ++i) {
		ResetBeamEvent(slots[i]);
	}
	size_t tofill_entry = 0;

	long long total = ipt->GetEntries();
	long long last_percentage = 0;
	if (report) {
		printf("Forging beam with trigger   0%%");
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
			if (std::fabs(*iter - raw.time) < min_time) {
				min_time = std::fabs(*iter - raw.time);
				min_time_entry = iter - trigger_time.begin();
			}
		}
		if (min_time > window) continue;
		if (min_time_entry < tofill_entry) continue;
		if (min_time_entry - tofill_entry >= kSlotNum) {
			for (size_t fill = tofill_entry; fill <= min_time_entry-kSlotNum; ++fill) {
				beam = slots[fill%kSlotNum];
				opt.Fill();
				ResetBeamEvent(slots[fill%kSlotNum]);
			}
			tofill_entry = min_time_entry - kSlotNum + 1;
		}
		UpdateBeamEvent(
			raw,
			slots[min_time_entry%kSlotNum]
		);
	}
	for (size_t fill = tofill_entry; fill < trigger_time.size(); ++fill) {
		beam = slots[fill%kSlotNum];
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
	bool report
) {
	TFile opf(output_path, "recreate");
	TH1F forge_window("fw", "forge window", 200, -window, window);
	TTree opt("tree", "forged beam");
	BeamEvent beam;
	SetupOutput(&opt, beam);

	TFile ipf(path, "read");
	TTree *ipt = (TTree*)ipf.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from " << path << " failed.\n";
		return -1;
	}
	RawBeamEvent raw;
	SetupInput(ipt, raw);

	ResetBeamEvent(beam);
	double ref_time = -1.0;
	long long total_entries = ipt->GetEntries();
	long long last_percentage = 0;
	if (report) {
		printf("Forging beam without trigger   0%%");
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
		if (beam.flag == 0) {
			UpdateBeamEvent(raw, beam);
			ref_time = raw.time;
		} else if (std::fabs(raw.time - ref_time) < window) {
			forge_window.Fill(raw.time - ref_time);
			UpdateBeamEvent(raw, beam);
		} else {
			opt.Fill();
			ResetBeamEvent(beam);
			UpdateBeamEvent(raw, beam);
			ref_time = raw.time;
		}
	}
	if (beam.flag != 0) opt.Fill();
	if (report) printf("\b\b\b\b100%%\n");

	opf.cd();
	forge_window.Write();
	opt.Write();
	ipf.Close();
	opf.Close();
	return 0;
}

} // namespace

int ForgeBeam(
	const std::vector<double> &trigger_time,
	const char *path,
	const char *output_path,
	const double window,
	bool report
) {
	if (trigger_time.empty()) {
		return ForgeWithoutTrigger(path, output_path, window, report);
	}
	return ForgeWithTrigger(trigger_time, path, output_path, window, report);
}

}
