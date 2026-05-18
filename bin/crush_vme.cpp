#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <string>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "external/cxxopts.hpp"
#include "include/config.h"
#include "include/crush/util.h"
#include "include/event/smelt/beam_event.h"
#include "include/event/smelt/csi_event.h"
#include "include/event/smelt/dssd_event.h"
#include "include/event/smelt/ppac_event.h"
#include "include/event/smelt/tafd_event.h"

namespace {

using namespace forgerib;

constexpr int kAdcModuleNum = 10;
constexpr int kAdcChannelNum = 32;
constexpr int kMadcModuleNum = 2;
constexpr int kMadcChannelNum = 32;
constexpr int kGdcBankNum = 2;
constexpr int kGdcChannelNum = 128;
constexpr int kGdcMultiNum = 5;
constexpr int kSdcChannelNum = 32;

struct ChannelRef {
	int module = -1;
	int channel = -1;

	bool Valid() const {
		return module >= 0 && channel >= 0;
	}
};

struct VmeBranches {
	int adc[kAdcModuleNum][kAdcChannelNum] = {};
	int madc[kMadcModuleNum][kMadcChannelNum] = {};
	int gdc[kGdcBankNum][kGdcChannelNum][kGdcMultiNum] = {};
	int gmulti[kGdcBankNum][kGdcChannelNum] = {};
	unsigned long long sdc[kSdcChannelNum] = {};
};

constexpr std::array<ChannelRef, 12> kTafCsi = {{
	{0, 15}, {0, 14}, {0, 13}, {0, 12}, {0, 11}, {0, 1},
	{0, 9}, {0, 8}, {0, 2}, {0, 6}, {0, 0}, {0, 4}
}};

constexpr std::array<int, 15> kPpacGdcChannels = {{
	16, 17, 24, 26,
	20, 21, 22, 23,
	31, 30, 29, 28,
	27, 19, 18
}};

ChannelRef TafdFrontChannel(int det, int strip) {
	return {det / 2, strip + (det % 2) * 16};
}

ChannelRef TafdBackChannel(int det, int strip) {
	return {3 + det / 4, strip + (det % 4) * 8};
}

ChannelRef T1FrontChannel(bool up, int strip) {
	if (up) return {5, strip};
	return {7, (strip + 16) % 32};
}

ChannelRef T1BackChannel(bool up, int strip) {
	return {up ? 6 : 8, strip};
}

ChannelRef T0CsiChannel(int index) {
	return {9, index < 14 ? index : 15};
}

ChannelRef T1dGaggChannel(int index) {
	return {4, index + 16};
}

ChannelRef T1uCsiChannel(int index) {
	return {0, index + 16};
}

void ResetTafdEvent(TafdEvent &event) {
	event.flag = 0;
	for (int i = 0; i < 6; ++i) {
		event.valid[i] = false;
		event.front_energy[i] = 0.0;
		event.back_energy[i] = 0.0;
		event.front_strip[i] = -1;
		event.back_strip[i] = -1;
		event.front_time[i] = 0.0;
	}
}

void ResetDssdEvent(DssdEvent &event) {
	event.front_num = 0;
	event.back_num = 0;
	for (int i = 0; i < 8; ++i) {
		event.front_strip[i] = -1;
		event.back_strip[i] = -1;
		event.front_energy[i] = 0.0;
		event.back_energy[i] = 0.0;
		event.front_time[i] = 0.0;
		event.back_time[i] = 0.0;
	}
}

void ResetCsiEvent(CsiEvent &event) {
	event.flag = 0;
	for (int i = 0; i < 36; ++i) {
		event.valid[i] = false;
		event.time[i] = 0.0;
		event.energy[i] = 0;
	}
}

void ResetBeamEvent(BeamEvent &event) {
	event.flag = 0;
	event.valid = false;
	event.tof = 0.0;
}

void ResetPpacEvent(PpacEvent &event) {
	event.flag = 0;
	for (int i = 0; i < 15; ++i) {
		event.valid[i] = false;
		event.time[i] = 0.0;
		event.energy[i] = 0;
	}
}

double GdcTime(const VmeBranches &input, int bank, int channel) {
	return double(input.gdc[bank][channel][0] - input.gdc[bank][0][0]) / 10.0;
}

int AdcEnergy(const VmeBranches &input, const ChannelRef &ref) {
	if (!ref.Valid()) return 0;
	if (ref.module >= kAdcModuleNum || ref.channel >= kAdcChannelNum) return 0;
	return input.adc[ref.module][ref.channel];
}

int MadcEnergy(const VmeBranches &input, const ChannelRef &ref) {
	if (!ref.Valid()) return 0;
	if (ref.module >= kMadcModuleNum || ref.channel >= kMadcChannelNum) return 0;
	return input.madc[ref.module][ref.channel];
}

void InsertDssdHit(
	int strip,
	int energy,
	double time,
	int &num,
	int strips[8],
	double energies[8],
	double times[8]
) {
	int pos = 0;
	while (pos < num && energies[pos] >= energy) ++pos;
	if (pos >= 8) return;
	int limit = std::min(num, 7);
	for (int i = limit; i > pos; --i) {
		strips[i] = strips[i-1];
		energies[i] = energies[i-1];
		times[i] = times[i-1];
	}
	strips[pos] = strip;
	energies[pos] = energy;
	times[pos] = time;
	if (num < 8) ++num;
}

bool BetterCandidate(int energy, int strip, int best_energy, int best_strip) {
	return energy > best_energy || (energy == best_energy && strip < best_strip);
}

void FillTafdEvent(const VmeBranches &input, TafdEvent &event) {
	ResetTafdEvent(event);
	for (int det = 0; det < 6; ++det) {
		int best_front_energy = -1;
		int best_front_strip = std::numeric_limits<int>::max();
		double best_front_time = 0.0;
		for (int strip = 0; strip < 16; ++strip) {
			ChannelRef ref = TafdFrontChannel(det, strip);
			int energy = AdcEnergy(input, ref);
			int time_channel = 32 + det * 16 + strip;
			if (energy <= 300 || energy >= 4096) continue;
			if (input.gmulti[0][time_channel] <= 0) continue;
			if (BetterCandidate(energy, strip, best_front_energy, best_front_strip)) {
				best_front_energy = energy;
				best_front_strip = strip;
				best_front_time = GdcTime(input, 0, time_channel);
			}
		}

		int best_back_energy = -1;
		int best_back_strip = std::numeric_limits<int>::max();
		for (int strip = 0; strip < 8; ++strip) {
			ChannelRef ref = TafdBackChannel(det, strip);
			int energy = AdcEnergy(input, ref);
			if (energy <= 300 || energy >= 4096) continue;
			if (BetterCandidate(energy, strip, best_back_energy, best_back_strip)) {
				best_back_energy = energy;
				best_back_strip = strip;
			}
		}

		if (best_front_energy > 0 && best_back_energy > 0) {
			event.valid[det] = true;
			event.flag |= 1 << det;
			event.front_energy[det] = best_front_energy;
			event.back_energy[det] = best_back_energy;
			event.front_strip[det] = best_front_strip;
			event.back_strip[det] = best_back_strip;
			event.front_time[det] = best_front_time;
		}
	}
}

void FillDssdEvent(
	const VmeBranches &input,
	DssdEvent &event,
	bool up
) {
	ResetDssdEvent(event);
	const int time_bank = 1;
	const int time_base = up ? 0 : 64;

	for (int strip = 0; strip < 32; ++strip) {
		ChannelRef front = T1FrontChannel(up, strip);
		int front_energy = AdcEnergy(input, front);
		if (front_energy >= 300 && input.gmulti[time_bank][time_base + strip] > 0) {
			InsertDssdHit(
				strip,
				front_energy,
				GdcTime(input, time_bank, time_base + strip),
				event.front_num,
				event.front_strip,
				event.front_energy,
				event.front_time
			);
		}

		ChannelRef back = T1BackChannel(up, strip);
		int back_energy = AdcEnergy(input, back);
		if (back_energy >= 300) {
			InsertDssdHit(
				strip,
				back_energy,
				0.0,
				event.back_num,
				event.back_strip,
				event.back_energy,
				event.back_time
			);
		}
	}
}

void UpdateCsiHit(CsiEvent &event, int index, int energy) {
	if (index < 0 || index >= 36 || energy <= 300 || energy >= 8192) return;
	if (!event.valid[index] || energy > event.energy[index]) {
		event.valid[index] = true;
		event.flag |= 1ull << index;
		event.energy[index] = energy;
		event.time[index] = 0.0;
	}
}

void FillT0CsiEvent(const VmeBranches &input, CsiEvent &event) {
	ResetCsiEvent(event);
	for (int i = 0; i < 15; ++i) {
		UpdateCsiHit(event, i, AdcEnergy(input, T0CsiChannel(i)));
	}
}

void FillTafCsiEvent(const VmeBranches &input, CsiEvent &event) {
	ResetCsiEvent(event);
	for (int i = 0; i < 12; ++i) {
		UpdateCsiHit(event, i, MadcEnergy(input, kTafCsi[i]));
	}
}

void FillT1DCsiEvent(const VmeBranches &input, CsiEvent &event) {
	ResetCsiEvent(event);
	for (int i = 0; i < 12; ++i) {
		UpdateCsiHit(event, i, AdcEnergy(input, T1dGaggChannel(i)));
	}
	UpdateCsiHit(event, 12, MadcEnergy(input, {0, 20}));
}

void FillT1UCsiEvent(const VmeBranches &input, CsiEvent &event) {
	ResetCsiEvent(event);
	for (int i = 0; i < 4; ++i) {
		UpdateCsiHit(event, i, MadcEnergy(input, T1uCsiChannel(i)));
	}
}

void FillBeamEvent(const VmeBranches &input, BeamEvent &event) {
	ResetBeamEvent(event);
	const int t1_channel = 97;
	const int t2_channel = 98;
	double t1 = 0.0;
	double t2 = 0.0;
	if (input.gmulti[1][t1_channel] > 0) {
		event.flag |= 1 << 0;
		t1 = GdcTime(input, 1, t1_channel);
	}
	if (input.gmulti[1][t2_channel] > 0) {
		event.flag |= 1 << 1;
		t2 = GdcTime(input, 1, t2_channel);
	}
	if ((event.flag & 0x3) == 0x3) {
		event.valid = true;
		event.tof = t2 - t1;
	}
}

void FillPpacEvent(const VmeBranches &input, PpacEvent &event) {
	ResetPpacEvent(event);
	for (int bit = 0; bit < 15; ++bit) {
		int channel = kPpacGdcChannels[bit];
		if (input.gmulti[0][channel] <= 0) continue;
		event.flag |= 1 << bit;
		event.valid[bit] = true;
		event.time[bit] = GdcTime(input, 0, channel);
	}
}

bool HasTafdTrigger(const TafdEvent &event, int threshold) {
	for (int i = 0; i < 6; ++i) {
		if (!event.valid[i]) continue;
		if (event.front_energy[i] > threshold && event.back_energy[i] > threshold) {
			return true;
		}
	}
	return false;
}

bool HasCsiTrigger(const CsiEvent &event, int threshold) {
	for (int i = 0; i < 36; ++i) {
		if (event.valid[i] && event.energy[i] > threshold) return true;
	}
	return false;
}

bool HasDssdTrigger(const DssdEvent &event) {
	return event.front_num > 0 || event.back_num > 0;
}

} // namespace

int main(int argc, char **argv) {
	cxxopts::Options options("crush_vme", "map VME ROOT file to crushed detector ROOT files");
	options.add_options()
		("h,help", "Print usage")
		(
			"v,vme_run",
			"VME run number, required.",
			cxxopts::value<int>()
		);
	auto parse_result = options.parse(argc, argv);
	if (parse_result.count("help") || !parse_result.count("vme_run")) {
		std::cout << options.help() << std::endl;
		return 0;
	}

	const int vme_run = parse_result["vme_run"].as<int>();

	AppConfig config;
	if (LoadConfig("config.toml", config)) {
		return -1;
	}
	const std::string output_dir = JoinPath(config.workspace, config.paths.grit);

	TString input_path = TString::Format(
		"%s/%s%04d.root",
		config.paths.raw_vme.c_str(),
		config.prefix.raw_vme.c_str(),
		vme_run
	);
	TFile input_file(input_path, "read");
	TTree *input_tree = static_cast<TTree*>(input_file.Get("tree"));
	if (!input_tree) {
		std::cerr << "Error: Get tree from " << input_path << " failed.\n";
		return -1;
	}

	VmeBranches input;
	input_tree->SetBranchAddress("adc", input.adc);
	input_tree->SetBranchAddress("madc", input.madc);
	input_tree->SetBranchAddress("gdc", input.gdc);
	input_tree->SetBranchAddress("gmulti", input.gmulti);
	input_tree->SetBranchAddress("sdc", input.sdc);

	TString tafd_path = TString::Format("%s/tafd_vme_%04d.root", output_dir.c_str(), vme_run);
	TFile tafd_file(tafd_path, "recreate");
	TTree tafd_tree("tree", "tafd vme");
	TafdEvent tafd_event;
	SetupOutput(&tafd_tree, tafd_event);

	TString t1du_path = TString::Format("%s/t1du_vme_%04d.root", output_dir.c_str(), vme_run);
	TFile t1du_file(t1du_path, "recreate");
	TTree t1du_tree("tree", "t1du vme");
	DssdEvent t1du_event;
	SetupOutput(&t1du_tree, t1du_event);

	TString t1dd_path = TString::Format("%s/t1dd_vme_%04d.root", output_dir.c_str(), vme_run);
	TFile t1dd_file(t1dd_path, "recreate");
	TTree t1dd_tree("tree", "t1dd vme");
	DssdEvent t1dd_event;
	SetupOutput(&t1dd_tree, t1dd_event);

	TString t0csi_path = TString::Format("%s/t0csi_vme_%04d.root", output_dir.c_str(), vme_run);
	TFile t0csi_file(t0csi_path, "recreate");
	TTree t0csi_tree("tree", "t0csi vme");
	CsiEvent t0csi_event;
	SetupOutput(&t0csi_tree, t0csi_event);

	TString tafcsi_path = TString::Format("%s/tafcsi_vme_%04d.root", output_dir.c_str(), vme_run);
	TFile tafcsi_file(tafcsi_path, "recreate");
	TTree tafcsi_tree("tree", "tafcsi vme");
	CsiEvent tafcsi_event;
	SetupOutput(&tafcsi_tree, tafcsi_event);

	TString t1dcsi_path = TString::Format("%s/t1csid_vme_%04d.root", output_dir.c_str(), vme_run);
	TFile t1dcsi_file(t1dcsi_path, "recreate");
	TTree t1dcsi_tree("tree", "t1dcsi vme");
	CsiEvent t1dcsi_event;
	SetupOutput(&t1dcsi_tree, t1dcsi_event);

	TString t1ucsi_path = TString::Format("%s/t1csiu_vme_%04d.root", output_dir.c_str(), vme_run);
	TFile t1ucsi_file(t1ucsi_path, "recreate");
	TTree t1ucsi_tree("tree", "t1ucsi vme");
	CsiEvent t1ucsi_event;
	SetupOutput(&t1ucsi_tree, t1ucsi_event);

	TString beam_path = TString::Format("%s/beam_vme_%04d.root", output_dir.c_str(), vme_run);
	TFile beam_file(beam_path, "recreate");
	TTree beam_tree("tree", "beam vme");
	BeamEvent beam_event;
	SetupOutput(&beam_tree, beam_event);

	TString ppac_path = TString::Format("%s/ppac_vme_%04d.root", output_dir.c_str(), vme_run);
	TFile ppac_file(ppac_path, "recreate");
	TTree ppac_tree("tree", "ppac vme");
	PpacEvent ppac_event;
	SetupOutput(&ppac_tree, ppac_event);

	TString trigger_path = TString::Format("%s/trigger_vme_%04d.root", output_dir.c_str(), vme_run);
	TFile trigger_file(trigger_path, "recreate");
	TTree trigger_tree("tree", "trigger vme");
	bool trigger_valid = true;
	long long trigger_time = 0;
	trigger_tree.Branch("valid", &trigger_valid, "v/O");
	trigger_tree.Branch("time", &trigger_time, "time/L");

	TString trigger_taf_path = TString::Format(
		"%s/trigger_vme_taf_%04d.root",
		output_dir.c_str(),
		vme_run
	);
	TFile trigger_taf_file(trigger_taf_path, "recreate");
	TTree trigger_taf_tree("tree", "trigger vme taf");
	bool trigger_taf_valid = false;
	long long trigger_taf_time = 0;
	trigger_taf_tree.Branch("valid", &trigger_taf_valid, "v/O");
	trigger_taf_tree.Branch("time", &trigger_taf_time, "time/L");

	TString trigger_t1_path = TString::Format(
		"%s/trigger_vme_t1_%04d.root",
		output_dir.c_str(),
		vme_run
	);
	TFile trigger_t1_file(trigger_t1_path, "recreate");
	TTree trigger_t1_tree("tree", "trigger vme t1");
	bool trigger_t1_valid = false;
	long long trigger_t1_time = 0;
	trigger_t1_tree.Branch("valid", &trigger_t1_valid, "v/O");
	trigger_t1_tree.Branch("time", &trigger_t1_time, "time/L");

	const long long total = input_tree->GetEntries();
	long long last_percentage = 0;
	printf("Mapping VME   0%%");
	fflush(stdout);
	for (long long entry = 0; entry < total; ++entry) {
		if (entry * 100ll / total > last_percentage) {
			last_percentage = entry * 100ll / total;
			printf("\b\b\b\b%3lld%%", last_percentage);
			fflush(stdout);
		}

		input_tree->GetEntry(entry);

		FillTafdEvent(input, tafd_event);
		FillDssdEvent(input, t1du_event, true);
		FillDssdEvent(input, t1dd_event, false);
		FillT0CsiEvent(input, t0csi_event);
		FillTafCsiEvent(input, tafcsi_event);
		FillT1DCsiEvent(input, t1dcsi_event);
		FillT1UCsiEvent(input, t1ucsi_event);
		FillBeamEvent(input, beam_event);
		FillPpacEvent(input, ppac_event);
		trigger_time = static_cast<long long>(input.sdc[1]);
		trigger_taf_time = trigger_time;
		trigger_t1_time = trigger_time;
		trigger_taf_valid =
			HasTafdTrigger(tafd_event, 300)
			|| HasCsiTrigger(tafcsi_event, 300)
			|| HasCsiTrigger(t0csi_event, 300);
		trigger_t1_valid =
			trigger_taf_valid
			|| HasDssdTrigger(t1du_event)
			|| HasDssdTrigger(t1dd_event)
			|| HasCsiTrigger(t1dcsi_event, 300)
			|| HasCsiTrigger(t1ucsi_event, 300);

		tafd_tree.Fill();
		t1du_tree.Fill();
		t1dd_tree.Fill();
		t0csi_tree.Fill();
		tafcsi_tree.Fill();
		t1dcsi_tree.Fill();
		t1ucsi_tree.Fill();
		beam_tree.Fill();
		ppac_tree.Fill();
		trigger_tree.Fill();
		trigger_taf_tree.Fill();
		trigger_t1_tree.Fill();
	}
	printf("\b\b\b\b100%%\n");

	tafd_file.cd();
	tafd_tree.Write();
	tafd_file.Close();

	t1du_file.cd();
	t1du_tree.Write();
	t1du_file.Close();

	t1dd_file.cd();
	t1dd_tree.Write();
	t1dd_file.Close();

	t0csi_file.cd();
	t0csi_tree.Write();
	t0csi_file.Close();

	tafcsi_file.cd();
	tafcsi_tree.Write();
	tafcsi_file.Close();

	t1dcsi_file.cd();
	t1dcsi_tree.Write();
	t1dcsi_file.Close();

	t1ucsi_file.cd();
	t1ucsi_tree.Write();
	t1ucsi_file.Close();

	beam_file.cd();
	beam_tree.Write();
	beam_file.Close();

	ppac_file.cd();
	ppac_tree.Write();
	ppac_file.Close();

	trigger_file.cd();
	trigger_tree.Write();
	trigger_file.Close();

	trigger_taf_file.cd();
	trigger_taf_tree.Write();
	trigger_taf_file.Close();

	trigger_t1_file.cd();
	trigger_t1_tree.Write();
	trigger_t1_file.Close();

	input_file.Close();
	return 0;
}
