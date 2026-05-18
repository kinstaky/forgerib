#include "include/smelt/smelt_trigger.h"

#include <iostream>
#include <cmath>
#include <algorithm>

#include <TFile.h>
#include <TH1F.h>
#include <TTree.h>

#include "include/event/ingot/trigger_event.h"
#include "include/event/ore/raw_trigger_event.h"

namespace forgerib {

constexpr size_t kSlotNum = 1000;

void ResetTriggerEvent(TriggerEvent &trigger) {
	trigger.flag = 0;
	for (int i = 0; i < 6; ++i) {
		trigger.valid[i] = false;
		trigger.time[i] = 0.0;
		trigger.external_time[i] = 0;
	}
}

void UpdateTriggerEvent(
	const RawTriggerEvent &raw,
	TriggerEvent &trigger
) {
	if (raw.type < 0 || raw.type >= 6) return;
	trigger.flag |= 1 << raw.type;
	trigger.valid[raw.type] = true;
	trigger.time[raw.type] = raw.time;
	trigger.external_time[raw.type] = raw.external_time;
}


int SmeltTrigger(
	const std::vector<double> &trigger_time,
	const char *path,
	const char *output_path,
	const double window,
	bool report
) {
	if (trigger_time.empty()) {
		std::cerr << "Error: Forge trigger requires main trigger reference.\n";
		return -1;
	}
	std::cout << "Trigger size: " << trigger_time.size() << "\n";

	TFile opf(output_path, "recreate");
	TH1F forge_window("fw", "forge window", 200, -window, window);
	TTree opt("tree", "forged trigger");
	TriggerEvent trigger;
	SetupOutput(&opt, trigger);

	TFile ipf(path, "read");
	TTree *ipt = (TTree*)ipf.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from "
			<< path << " failed.\n";
		return -1;
	}
	RawTriggerEvent raw;
	SetupInput(ipt, raw);

	TriggerEvent slots[kSlotNum];
	for (size_t i = 0; i < kSlotNum; ++i) {
		ResetTriggerEvent(slots[i]);
	}
	size_t tofill_entry = 0;

	long long total = ipt->GetEntries();
	long long last_percentage = 0;
	if (report) {
		printf("Forging trigger   0%%");
		fflush(stdout);
	}
	for (long long entry = 0; entry < total; ++entry) {
		if (report && entry * 100ll / total > last_percentage) {
			last_percentage = entry * 100ll / total;
			printf("\b\b\b\b%3lld%%", last_percentage);
			fflush(stdout);
		}
		ipt->GetEntry(entry);
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
		//printf("Type: %d, time: %f, entry %ld, tofill %ld, slot_flag %x\n", raw.type, raw.time, min_time_entry, tofill_entry, slots[min_time_entry%kSlotNum].flag);
		if (min_time_entry < tofill_entry) continue;
		if (min_time_entry - tofill_entry >= kSlotNum) {
			for (size_t fill = tofill_entry; fill <= min_time_entry-kSlotNum; ++fill) {
				trigger = slots[fill%kSlotNum];
				opt.Fill();
				ResetTriggerEvent(slots[fill%kSlotNum]);
			}
			tofill_entry = min_time_entry - kSlotNum+1;
		}
		UpdateTriggerEvent(raw, slots[min_time_entry%kSlotNum]);
	}
	for (size_t fill = tofill_entry; fill < trigger_time.size(); ++fill) {
		trigger = slots[fill%kSlotNum];
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

}