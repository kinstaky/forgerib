#include "include/smelt/smelt_detector.h"

#include <iostream>

#include <TFile.h>
#include <TTree.h>

#include "include/event/smelt/csi_event.h"
#include "include/event/smelt/align_event.h"

namespace forgerib {

int SmeltDetectorWithXiaTrigger(
	const std::vector<AlignEvent> &align_events,
	const long long xia_entries,
	const char *vme_path,
	const char *output_path,
	bool report
) {
	TFile vme_file(vme_path, "read");
	TTree *vme_tree = (TTree*)vme_file.Get("tree");
	if (!vme_tree) {
		std::cerr << "Error: Get tree from " << vme_path << " failed.\n";
		return -2;
	}
	CsiEvent vme_csi;
	SetupInput(vme_tree, vme_csi);

	TFile opf(output_path, "recreate");
	TTree opt("tree", "fused T1 DSSD");
	CsiEvent csi;
	SetupOutput(&opt, csi);


	if (report) {
		printf("Fusing t1csi   0%%");
		fflush(stdout);
	}
	long long last_percentage = 0;
	auto align_iter = align_events.begin();
	for (long long entry = 0; entry < xia_entries; ++entry) {
		if (report && entry * 100ll / xia_entries > last_percentage) {
			last_percentage = entry * 100 / xia_entries;
			printf("\b\b\b\b%3lld%%", last_percentage);
			fflush(stdout);
		}
		// fuse vme events
		while (align_iter->xia_entry < entry && align_iter != align_events.end()) {
			++align_iter;
		}
		if (align_iter != align_events.end() && align_iter->xia_entry == entry) {
			vme_tree->GetEntry(align_iter->vme_entry);
			csi = vme_csi;
		}
		opt.Fill();
	}
	if (report) printf("\b\b\b\b100%%\n");

	opf.cd();
	opt.Write();
	opf.Close();
	vme_file.Close();
	return 0;
}


int SmeltDetectorWithVmeTrigger(
	const std::vector<AlignEvent> &align_events,
	const char *vme_path,
	const char *output_path,
	bool report
) {
	TFile vme_file(vme_path, "read");
	TTree *vme_tree = (TTree*)vme_file.Get("tree");
	if (!vme_tree) {
		std::cerr << "Error: Get tree from " << vme_path << " failed.\n";
		return -2;
	}
	CsiEvent vme_event;
	SetupInput(vme_tree, vme_event);

	TFile opf(output_path, "recreate");
	TTree opt("tree", "fused t0csi");
	CsiEvent event;
	SetupOutput(&opt, event);

	if (report) {
		printf("Fusing t1csi   0%%");
		fflush(stdout);
	}
	size_t last_percentage = 0;
	for (size_t i = 0; i < align_events.size(); ++i) {
		if (report && i * 100 / align_events.size() >last_percentage) {
			last_percentage = i * 100 / align_events.size();
			printf("\b\b\b\b%3lu%%", last_percentage);
			fflush(stdout);
		}
		// fuse vme events
		vme_tree->GetEntry(align_events[i].vme_entry);
		event = vme_event;
		opt.Fill();
	}
	if (report) printf("\b\b\b\b100%%\n");

	opf.cd();
	opt.Write();
	vme_file.Close();
	opf.Close();
	return 0;
}
}