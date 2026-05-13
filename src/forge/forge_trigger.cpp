#include "include/forge/forge_trigger.h"

#include <iostream>

#include <TFile.h>
#include <TH1F.h>
#include <TTree.h>

#include "include/event/forge/trigger_event.h"
#include "include/event/raw/raw_trigger_event.h"

namespace glimmer {

namespace {

void ResetTriggerEvent(TriggerEvent &trigger) {
	trigger.flag = 0;
	for (int i = 0; i < 6; ++i) {
		trigger.valid[i] = false;
		trigger.time[i] = 0.0;
	}
}

void UpdateTriggerEvent(
	const RawTriggerEvent &raw,
	const double ref_time,
	TriggerEvent &trigger
) {
	if (raw.type < 0 || raw.type >= 6) return;
	trigger.flag |= 1 << raw.type;
	trigger.valid[raw.type] = true;
	trigger.time[raw.type] = raw.time;
}

} // namespace

int ForgeTrigger(
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

	RawTriggerEvent buffer[10];
	size_t buffer_top = 0;
	size_t buffer_size = 0;
	size_t total = trigger_time.size();
	size_t last_percentage = 0;
	if (report) {
		printf("Forging trigger   0%%");
		fflush(stdout);
	}
	long long entry = 0;
	for (size_t i = 0; i < trigger_time.size(); ++i) {
		if (report && i * 100 / total > last_percentage) {
			last_percentage = i * 100 / total;
			printf("\b\b\b\b%3lu%%", last_percentage);
			fflush(stdout);
		}
		const double &ref_time = trigger_time[i];
		ResetTriggerEvent(trigger);
		//while (buffer_top < buffer_size) {
		//	if (buffer[buffer_top])
		//}
		while (entry < ipt->GetEntriesFast()) {
			ipt->GetEntry(entry);
			if (!raw.cv) {
				++entry;
				continue;
			}
			if (raw.time < ref_time - window) {
				++entry;
				continue;
			}
			if (raw.time <= ref_time + window) {
				forge_window.Fill(raw.time - ref_time);
				UpdateTriggerEvent(raw, ref_time, trigger);
				++entry;
				continue;
			}
			break;
		}
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
