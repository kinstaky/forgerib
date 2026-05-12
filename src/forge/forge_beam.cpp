#include "include/forge/forge_beam.h"

#include <cmath>
#include <iostream>

#include <TFile.h>
#include <TH1F.h>
#include <TTree.h>

#include "include/event/forge/beam_event.h"
#include "include/event/raw/raw_beam_event.h"

namespace glimmer {

namespace {

void ResetBeamEvent(BeamEvent &beam) {
	beam.flag = 0;
	beam.valid = false;
	beam.tof = 0.0;
}

void UpdateBeamEvent(
	const RawBeamEvent &raw,
	BeamEvent &beam,
	double type_time[3],
	bool seen[3]
) {
	if (raw.type < 0 || raw.type > 2) return;
	beam.flag |= 1 << raw.type;
	if (!seen[raw.type]) {
		seen[raw.type] = true;
		type_time[raw.type] = raw.time;
	}
	if (seen[kBeamT1] && seen[kBeamT2]) {
		beam.valid = true;
		beam.tof = type_time[kBeamT2] - type_time[kBeamT1];
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

	size_t total = trigger_time.size();
	size_t last_percentage = 0;
	if (report) {
		printf("Forging beam with trigger   0%%");
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
		ResetBeamEvent(beam);
		double type_time[3] = {0.0, 0.0, 0.0};
		bool seen[3] = {false, false, false};
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
			if (raw.time < ref_time + window) {
				forge_window.Fill(raw.time - ref_time);
				UpdateBeamEvent(raw, beam, type_time, seen);
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
	double type_time[3] = {0.0, 0.0, 0.0};
	bool seen[3] = {false, false, false};
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
			UpdateBeamEvent(raw, beam, type_time, seen);
			ref_time = raw.time;
		} else if (std::fabs(raw.time - ref_time) < window) {
			forge_window.Fill(raw.time - ref_time);
			UpdateBeamEvent(raw, beam, type_time, seen);
		} else {
			opt.Fill();
			ResetBeamEvent(beam);
			type_time[0] = 0.0;
			type_time[1] = 0.0;
			type_time[2] = 0.0;
			seen[0] = false;
			seen[1] = false;
			seen[2] = false;
			UpdateBeamEvent(raw, beam, type_time, seen);
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
