#include <iostream>

#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TParameter.h>

#include "include/fuse/fuse_t0csi.h"
#include "include/fuse/fuse_detector.h"
#include "include/event/forge/dssd_event.h"
#include "include/event/forge/tafd_event.h"
#include "include/event/forge/csi_event.h"
#include "include/event/forge/align_event.h"

#include "external/cxxopts.hpp"
#include "external/toml.hpp"

using namespace glimmer;

int ReadAlignEvents(
	const char *workspace,
	const std::string &trigger_type,
	const int run,
	std::vector<AlignEvent> &events,
	bool report = false
) {
	TString filename = TString::Format(
		"%s/forge/align_%s%04d.root",
		workspace,
		trigger_type == "main" ? "" : (trigger_type+"_").c_str(),
		run
	);
	TFile ipf(filename, "read");
	TTree *ipt = (TTree*)ipf.Get("tree");
	if (!ipt) {
		std::cerr << "Error: Get tree from "
			<< filename << " failed.\n";
		return -1;
	}
	glimmer::AlignEvent align_event;
	glimmer::SetupInput(ipt, align_event);

	events.clear();
	if (report) {
		printf("Reading align events   0%%");
		fflush(stdout);
	}
	long long total = ipt->GetEntries();
	long long last_percentage = 0;
	for (long long entry = 0; entry < total; ++entry) {
		if (report && entry * 100ll / total > last_percentage) {
			last_percentage = entry * 100ll / total;
			printf("\b\b\b\b%3lld%%", last_percentage);
			fflush(stdout);
		}
		ipt->GetEntry(entry);
		events.push_back(align_event);
	}
	if (report) {
		printf("\b\b\b\b100%%\n");
	}
	ipf.Close();
	return 0;
}


long long GetXiaTotal(const char *workspace, int run) {
	TString filename = TString::Format(
		"%s/forge/trigger_%04d.root",
		workspace,
		run
	);
	TFile ipf(filename, "read");
	TTree *ipt = (TTree*)ipf.Get("tree");
	long long entries = ipt->GetEntries();
	ipf.Close();
	return entries;
}


int main(int argc, char **argv) {
	cxxopts::Options options("forge", "forge detectors");
	options.add_options()
		("h,help", "Print usage")
		(
			"t,trigger",
			"Use main trigger.",
			cxxopts::value<std::string>()->default_value("main"),
			"trigger"
		)
		(
			"x,xia_run",
			"XIA run number, required.",
			cxxopts::value<int>(),
			"run"
		)
		(
			"v,vme_run",
			"VME run number, requried.",
			cxxopts::value<int>(),
			"run"
		)
		(
			"detectors",
			"Detector to fuse: t0csi, t1du, t1dd, t1csiu, t1csid",
			cxxopts::value<std::vector<std::string>>(),
			"detector"
		);
	options.parse_positional({"detectors"});
	auto parse_result = options.parse(argc, argv);

	if (
		parse_result.count("help")
		|| !parse_result.count("xia_run")
		|| !parse_result.count("vme_run")
	) {
		std::cout << options.help() << std::endl;
		return 0;
	}
	std::string trigger_type = parse_result["trigger"].as<std::string>();
	int xia_run = parse_result["xia_run"].as<int>();
	int vme_run = parse_result["vme_run"].as<int>();
	std::vector<std::string> detectors =
		parse_result["detectors"].as<std::vector<std::string>>();
	if (detectors.empty()) {
		std::cout << options.help() << std::endl;
		return 0;
	}

	toml::table tbl = toml::parse_file("config.toml");
	if (!tbl.contains("workspace") || !tbl["workspace"].is_string()) {
		std::cerr << "Error: Missing required parameter workspace from config.toml.\n";
		return -1;
	}
	std::string workspace = tbl["workspace"].as_string()->get();

	std::vector<AlignEvent> align_events;
	if (ReadAlignEvents(
		workspace.c_str(),
		trigger_type,
		xia_run,
		align_events,
		true
	)) {
		std::cerr << "Error: Read align events failed.\n";
		return -2;
	}
	long long xia_total_entries = GetXiaTotal(workspace.c_str(), xia_run);


	for (const std::string &det : detectors) {
		if (det == "t0csi") {
			TString xia_csi_path = TString::Format(
				"%s/forge/t0csi_%s%04d.root",
				workspace.c_str(),
				trigger_type == "main" ? "" : (trigger_type+("_")).c_str(),
				xia_run
			);
			TString vme_csi_path = TString::Format(
				"%s/forge/t0csi_vme_%04d.root",
				workspace.c_str(),
				vme_run
			);
			TString output_path = TString::Format(
				"%s/fuse/t0csi_%s%04d.root",
				workspace.c_str(),
				trigger_type == "main" ? "" : (trigger_type+("_")).c_str(),
				xia_run
			);
			int result = 0;
			if (trigger_type == "main") {
				result = glimmer::FuseT0CsIWithXiaTrigger(
					align_events,
					xia_csi_path,
					vme_csi_path,
					output_path,
					true
				);
			} else {
				result = glimmer::FuseT0CsIWithVmeTrigger(
					align_events,
					xia_csi_path,
					vme_csi_path,
					output_path,
					true
				);
			}
			if (result) {
				std::cerr << "Error: Fuse T0CsI failed.\n";
			} else {
				std::cout << "Fuse T0CsI success!\n";
			}
		} else if (det == "t1du" || det == "t1ud") {
			TString vme_path = TString::Format(
				"%s/forge/t1du_vme_%04d.root",
				workspace.c_str(),
				vme_run
			);
			TString output_path = TString::Format(
				"%s/fuse/t1du_%s%04d.root",
				workspace.c_str(),
				trigger_type == "main" ? "" : (trigger_type+("_")).c_str(),
				xia_run
			);
			int result = 0;
			if (trigger_type == "main") {
				result = glimmer::FuseDetectorWithXiaTrigger<DssdEvent>(
					align_events,
					xia_total_entries,
					vme_path,
					output_path,
					"t1du",
					true
				);
			} else {
				result = glimmer::FuseDetectorWithVmeTrigger<DssdEvent>(
					align_events,
					vme_path,
					output_path,
					"t1du",
					true
				);
			}
			if (result) {
				std::cerr << "Error: Fuse T1 DSSD up failed.\n";
			} else {
				std::cout << "Fuse T1 DSSD up success!\n";
			}
		} else if (det == "t1dd") {
			TString vme_path = TString::Format(
				"%s/forge/t1dd_vme_%04d.root",
				workspace.c_str(),
				vme_run
			);
			TString output_path = TString::Format(
				"%s/fuse/t1dd_%s%04d.root",
				workspace.c_str(),
				trigger_type == "main" ? "" : (trigger_type+("_")).c_str(),
				xia_run
			);
			int result = 0;
			if (trigger_type == "main") {
				result = glimmer::FuseDetectorWithXiaTrigger<DssdEvent>(
					align_events,
					xia_total_entries,
					vme_path,
					output_path,
					"t1dd",
					true
				);
			} else {
				result = glimmer::FuseDetectorWithVmeTrigger<DssdEvent>(
					align_events,
					vme_path,
					output_path,
					"t1dd",
					true
				);
			}
			if (result) {
				std::cerr << "Error: Fuse T1 DSSD down failed.\n";
			} else {
				std::cout << "Fuse T1 DSSD down success!\n";
			}
		} else if (det == "t1csiu") {
			TString vme_path = TString::Format(
				"%s/forge/t1csiu_vme_%04d.root",
				workspace.c_str(),
				vme_run
			);
			TString output_path = TString::Format(
				"%s/fuse/t1csiu_%s%04d.root",
				workspace.c_str(),
				trigger_type == "main" ? "" : (trigger_type+("_")).c_str(),
				xia_run
			);
			int result = 0;
			if (trigger_type == "main") {
				result = glimmer::FuseDetectorWithXiaTrigger<CsiEvent>(
					align_events,
					xia_total_entries,
					vme_path,
					output_path,
					"t1csiu",
					true
				);
			} else {
				result = glimmer::FuseDetectorWithVmeTrigger<CsiEvent>(
					align_events,
					vme_path,
					output_path,
					"t1csiu",
					true
				);
			}
			if (result) {
				std::cerr << "Error: Fuse T1 CsI up failed.\n";
			} else {
				std::cout << "Fuse T1 CsI up success!\n";
			}
		} else if (det == "t1csid") {
			TString vme_path = TString::Format(
				"%s/forge/t1csid_vme_%04d.root",
				workspace.c_str(),
				vme_run
			);
			TString output_path = TString::Format(
				"%s/fuse/t1csid_%s%04d.root",
				workspace.c_str(),
				trigger_type == "main" ? "" : (trigger_type+("_")).c_str(),
				xia_run
			);
			int result = 0;
			if (trigger_type == "main") {
				result = glimmer::FuseDetectorWithXiaTrigger<CsiEvent>(
					align_events,
					xia_total_entries,
					vme_path,
					output_path,
					"t1csid",
					true
				);
			} else {
				result = glimmer::FuseDetectorWithVmeTrigger<CsiEvent>(
					align_events,
					vme_path,
					output_path,
					"t1csid",
					true
				);
			}
			if (result) {
				std::cerr << "Error: Fuse T1 CsI down failed.\n";
			} else {
				std::cout << "Fuse T1 CsI down success!\n";
			}
		} else if (det == "tafd") {
			if (trigger_type != "main") continue;
			TString vme_path = TString::Format(
				"%s/forge/tafd_vme_%04d.root",
				workspace.c_str(),
				vme_run
			);
			TString output_path = TString::Format(
				"%s/fuse/tafd_%s%04d.root",
				workspace.c_str(),
				trigger_type == "main" ? "" : (trigger_type+("_")).c_str(),
				xia_run
			);
			int result = 0;
			if (trigger_type == "main") {
				result = glimmer::FuseDetectorWithXiaTrigger<TafdEvent>(
					align_events,
					xia_total_entries,
					vme_path,
					output_path,
					"tafd",
					true
				);
			} else {
				result = glimmer::FuseDetectorWithVmeTrigger<TafdEvent>(
					align_events,
					vme_path,
					output_path,
					"tafd",
					true
				);
			}
			if (result) {
				std::cerr << "Error: Fuse TAFD failed.\n";
			} else {
				std::cout << "Fuse TAFD success!\n";
			}
		}
	}

	return 0;
}
