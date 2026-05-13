#include "include/forge/forge_csi.h"

#include <iostream>

#include <TFile.h>
#include <TH1F.h>
#include <TTree.h>

#include "include/event/forge/csi_event.h"
#include "include/event/raw/raw_csi_event.h"

namespace glimmer {

namespace {

void ResetCsiEvent(CsiEvent &csi) {
	csi.flag = 0;
	for (int i = 0; i < 36; ++i) {
		csi.valid[i] = false;
		csi.time[i] = 0.0;
		csi.energy[i] = 0;
	}
}

void UpdateCsiEvent(const RawCsiEvent &raw, CsiEvent &csi) {
	if (raw.index < 0 || raw.index >= 36) return;
	if (!csi.valid[raw.index] || raw.energy > csi.energy[raw.index]) {
		csi.valid[raw.index] = true;
		csi.flag |= 1ull << raw.index;
		csi.time[raw.index] = raw.time;
		csi.energy[raw.index] = raw.energy;
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
	TTree opt("tree", "forged csi");
	CsiEvent csi;
	SetupOutput(&opt, csi);

	TFile ipf(path, "read");
	TTree *ipt = (TTree*)ipf.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from " << path << " failed.\n";
		return -1;
	}
	RawCsiEvent raw;
	SetupInput(ipt, raw);

	size_t total = trigger_time.size();
	size_t last_percentage = 0;
	if (report) {
		printf("Forging csi with trigger   0%%");
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
		ResetCsiEvent(csi);
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
				UpdateCsiEvent(raw, csi);
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

int ForgeWithoutTrigger(
	const char *path,
	const char *output_path,
	const double window,
	bool report
) {
	TFile opf(output_path, "recreate");
	TH1F forge_window("fw", "forge window", 200, -window, window);
	TTree opt("tree", "forged csi");
	CsiEvent csi;
	SetupOutput(&opt, csi);

	TFile ipf(path, "read");
	TTree *ipt = (TTree*)ipf.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from " << path << " failed.\n";
		return -1;
	}
	RawCsiEvent raw;
	SetupInput(ipt, raw);

	ResetCsiEvent(csi);
	double ref_time = -1.0;
	long long total_entries = ipt->GetEntriesFast();
	long long last_percentage = 0;
	if (report) {
		printf("Forging csi without trigger   0%%");
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
		if (csi.flag == 0) {
			UpdateCsiEvent(raw, csi);
			ref_time = raw.time;
		} else if (raw.time - ref_time < window) {
			forge_window.Fill(raw.time - ref_time);
			UpdateCsiEvent(raw, csi);
		} else {
			opt.Fill();
			ResetCsiEvent(csi);
			UpdateCsiEvent(raw, csi);
			ref_time = raw.time;
		}
	}
	if (csi.flag != 0) opt.Fill();
	if (report) printf("\b\b\b\b100%%\n");

	opf.cd();
	forge_window.Write();
	opt.Write();
	ipf.Close();
	opf.Close();
	return 0;
}

} // namespace

int ForgeCsi(
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
