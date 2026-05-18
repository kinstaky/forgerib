#include "include/smelt/smelt_t0csi.h"

#include <iostream>

#include <TFile.h>
#include <TTree.h>

#include "include/event/smelt/csi_event.h"
#include "include/event/smelt/align_event.h"

namespace forgerib {

int SmeltT0CsiWithXiaTrigger(
	const std::vector<AlignEvent> &align_events,
	const char *xia_csi_path,
	const char *vme_csi_path,
	const char *output_path,
	bool report
) {
	TFile xia_csi_file(xia_csi_path, "read");
	TTree *xia_tree = (TTree*)xia_csi_file.Get("tree");
	if (!xia_tree) {
		std::cerr << "Error: Get tree from " << xia_csi_path << " failed.\n";
		return -1;
	}
	CsiEvent xia_csi;
	SetupInput(xia_tree, xia_csi);

	TFile vme_csi_file(vme_csi_path, "read");
	TTree *vme_tree = (TTree*)vme_csi_file.Get("tree");
	if (!vme_tree) {
		std::cerr << "Error: Get tree from " << vme_csi_path << " failed.\n";
		return -2;
	}
	CsiEvent vme_csi;
	SetupInput(vme_tree, vme_csi);

	TFile opf(output_path, "recreate");
	TTree opt("tree", "fused t0csi");
	CsiEvent csi;
	SetupOutput(&opt, csi);

	if (report) {
		printf("Fusing t0csi   0%%");
		fflush(stdout);
	}
	long long total = xia_tree->GetEntries();
	long long last_percentage = 0;
	auto align_iter = align_events.begin();
	for (long long entry = 0; entry < total; ++entry) {
		if (report && entry * 100ll / total > last_percentage) {
			last_percentage = entry * 100 / total;
			printf("\b\b\b\b%3lld%%", last_percentage);
			fflush(stdout);
		}
		// fuse xia events
		xia_tree->GetEntry(entry);
		Reset(csi);
		for (int i = 15; i < 36; ++i) {
			if (!xia_csi.valid[i]) continue;
			Update(csi, i, xia_csi.energy[i], xia_csi.time[i]);
			// std::cout << "Found XIA csi: " << i << ", " << xia_csi.energy[i] << ", " << xia_csi.time[i] << "\n";
		}
		// fuse vme events
		while (align_iter != align_events.end() && align_iter->xia_entry < entry) {
			++align_iter;
		}
		if (align_iter != align_events.end() && align_iter->xia_entry == entry) {
			vme_tree->GetEntry(align_iter->vme_entry);
			for (int i = 0; i < 15; ++i) {
				if (!vme_csi.valid[i]) continue;
				Update(csi, i, vme_csi.energy[i], vme_csi.time[i]);
				// std::cout << "Found VME csi: " << i << ", " << vme_csi.energy[i] << ", " << vme_csi.time[i] << "\n";
			}
		}
		opt.Fill();
	}
	if (report) printf("\b\b\b\b100%%\n");

	opf.cd();
	opt.Write();
	opf.Close();
	vme_csi_file.Close();
	xia_csi_file.Close();
	return 0;
}


int SmeltT0CsiWithVmeTrigger(
	const std::vector<AlignEvent> &align_events,
	const char *xia_csi_path,
	const char *vme_csi_path,
	const char *output_path,
	bool report
) {
	TFile xia_csi_file(xia_csi_path, "read");
	TTree *xia_tree = (TTree*)xia_csi_file.Get("tree");
	if (!xia_tree) {
		std::cerr << "Error: Get tree from " << xia_csi_path << " failed.\n";
		return -1;
	}
	CsiEvent xia_csi;
	SetupInput(xia_tree, xia_csi);

	TFile vme_csi_file(vme_csi_path, "read");
	TTree *vme_tree = (TTree*)vme_csi_file.Get("tree");
	if (!vme_tree) {
		std::cerr << "Error: Get tree from " << vme_csi_path << " failed.\n";
		return -2;
	}
	CsiEvent vme_csi;
	SetupInput(vme_tree, vme_csi);

	TFile opf(output_path, "recreate");
	TTree opt("tree", "fused t0csi");
	CsiEvent csi;
	SetupOutput(&opt, csi);

	if (report) {
		printf("Fusing t0csi   0%%");
		fflush(stdout);
	}
	size_t last_percentage = 0;
	for (size_t i = 0; i < align_events.size(); ++i) {
		if (report && i * 100 / align_events.size() >last_percentage) {
			last_percentage = i * 100 / align_events.size();
			printf("\b\b\b\b%3lu%%", last_percentage);
			fflush(stdout);
		}
		// fuse xia events
		xia_tree->GetEntry(i);
		Reset(csi);
		for (int i = 15; i < 36; ++i) {
			if (!xia_csi.valid[i]) continue;
			Update(csi, i, xia_csi.energy[i], xia_csi.time[i]);
		}
		// fuse vme events
		vme_tree->GetEntry(align_events[i].vme_entry);
		for (int i = 0; i < 15; ++i) {
			if (!vme_csi.valid[i]) continue;
			Update(csi, i, vme_csi.energy[i], vme_csi.time[i]);
		}
		opt.Fill();
	}
	if (report) printf("\b\b\b\b100%%\n");

	opf.cd();
	opt.Write();
	opf.Close();
	vme_csi_file.Close();
	xia_csi_file.Close();
	return 0;
}
}
