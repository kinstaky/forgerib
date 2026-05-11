#include "include/forge/forge_ppac.h"

#include <iostream>
#include <cmath>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>

#include "include/event/forge/ppac_event.h"
#include "include/event/raw/raw_ppac_event.h"

namespace glimmer {

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

	size_t total = trigger_time.size();
	size_t last_percentage = 0;
	if (report) {
		printf("Forging PPAC with trigger   0%%");
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
		ppac.flag = 0;
		while (entry < ipt->GetEntriesFast()) {
			ipt->GetEntry(entry);
			if (!raw.cv) {
				++entry;
				continue;
			}
			int bit = raw.channel == 4
				? raw.index + 12
				: raw.index*4 + raw.channel;
			if (raw.time < ref_time - window) {
				++entry;
				continue;
			}
			if (raw.time < ref_time + window) {
				forge_window.Fill(raw.time - ref_time);
				if (
					(ppac.flag & (1 << bit)) == 0
					|| raw.energy > ppac.energy[bit]
				) {
					ppac.flag |= 1 << bit;
					ppac.time[bit] = raw.time;
					ppac.energy[bit] = raw.energy;
				}
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
