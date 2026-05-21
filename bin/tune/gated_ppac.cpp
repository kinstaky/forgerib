#include <cmath>
#include <iostream>

#include <TApplication.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH2F.h>
#include <TLine.h>
#include <TROOT.h>
#include <TString.h>
#include <TTree.h>

#include "include/config.h"
#include "include/util.h"
#include "include/event/smelt/beam_event.h"
#include "include/event/smelt/ppac_event.h"

namespace {

double kTofMin = 56.0;
double kTofMax = 66.0;
constexpr double kPpacZ[3] = {-52.0, -332.0, -612.0};

bool HasBit(int flag, int bit) {
	return (flag & (1 << bit)) != 0;
}

bool ComputePpacPosition(
	const forgerib::PpacEvent &ppac,
	int base_bit,
	int anode_bit,
	double &x,
	double &y
) {
	if (!HasBit(ppac.flag, anode_bit)) return false;
	if (!HasBit(ppac.flag, base_bit) || !HasBit(ppac.flag, base_bit + 1)) return false;
	if (!HasBit(ppac.flag, base_bit + 2) || !HasBit(ppac.flag, base_bit + 3)) return false;
	x = (ppac.time[base_bit] - ppac.time[base_bit + 1]) / 4.0;
	y = (ppac.time[base_bit + 2] - ppac.time[base_bit + 3]) / 4.0;
	return true;
}

bool LinearRegression(
	const double *z,
	const double *value,
	const bool *valid,
	double &intercept,
	double &slope
) {
	double sum_z = 0.0;
	double sum_v = 0.0;
	double sum_zv = 0.0;
	double sum_z2 = 0.0;
	int count = 0;
	for (int i = 0; i < 3; ++i) {
		if (!valid[i]) continue;
		sum_z += z[i];
		sum_v += value[i];
		sum_zv += z[i] * value[i];
		sum_z2 += z[i] * z[i];
		++count;
	}
	if (count < 2) return false;
	double denom = count * sum_z2 - sum_z * sum_z;
	if (std::fabs(denom) < 1e-9) return false;
	slope = (count * sum_zv - sum_z * sum_v) / denom;
	intercept = sum_v / count - sum_z / count * slope;
	return true;
}

void FillLine(TH2F &hist, double intercept, double slope) {
	for (double z = -800.0; z <= 100.0; z += 3.0) {
		hist.Fill(z, slope * z + intercept);
	}
}

} // namespace

int main(int argc, char **argv) {
	if (argc != 2 && argc != 4) {
		std::cout << "Usage: " << argv[0] << " [run] [min] [max].\n";
		return 1;
	}
	const int run = atoi(argv[1]);
	if (argc == 4) {
		kTofMin = double(atoi(argv[2]));
		kTofMax = double(atoi(argv[3]));
	}

	forgerib::AppConfig config;
	if (forgerib::LoadConfig("config.toml", config)) {
		return -1;
	}
	std::string ingot_dir = forgerib::JoinPath(config.workspace, config.paths.ingot);

	TString ppac_path = TString::Format("%s/ppac_%04d.root", ingot_dir.c_str(), run);
	TString beam_path = TString::Format("%s/beam_%04d.root", ingot_dir.c_str(), run);

	TFile ppac_file(ppac_path, "read");
	TTree *tree = (TTree*)ppac_file.Get("tree");
	if (!tree) {
		std::cerr << "Error: Get tree from " << ppac_path << " failed.\n";
		return -1;
	}
	//tree->AddFriend("tree", beam_path, "beam");

	TFile beam_file(beam_path, "read");
	TTree *beam_tree = (TTree*)beam_file.Get("tree");
	if (!beam_tree) {
		std::cerr << "Error: Get tree from " << beam_path << " failed.\n";
		return -1;
	}

	forgerib::PpacEvent ppac;
	forgerib::BeamEvent beam;
	forgerib::SetupInput(tree, ppac, "");
	forgerib::SetupInput(beam_tree, beam, "");

	int app_argc = 1;
	char app_name[] = "gated_ppac";
	char *app_argv[] = {app_name, nullptr};
	TApplication app(app_name, &app_argc, app_argv);
	gROOT->SetBatch(false);

	TCanvas canvas("canvas", "Gated PPAC", 1200, 800);
	canvas.Divide(3, 2);

	TH2F hist_ppac1("ppac1", "PPAC1", 50, -50, 50, 50, -50, 50);
	TH2F hist_ppac2("ppac2", "PPAC2", 50, -50, 50, 50, -50, 50);
	TH2F hist_ppac3("ppac3", "PPAC3", 50, -50, 50, 50, -50, 50);
	TH2F hist_target("target", "Target", 50, -50, 50, 50, -50, 50);
	TH2F hist_xz("xztrace", "X-Z Trace", 300, -800, 100, 100, -50, 50);
	TH2F hist_yz("yztrace", "Y-Z Trace", 300, -800, 100, 100, -50, 50);

	TLine lppac1(kPpacZ[0], -25, kPpacZ[0], 25);
	TLine lppac2(kPpacZ[1], -25, kPpacZ[1], 25);
	TLine lppac3(kPpacZ[2], -25, kPpacZ[2], 25);
	TLine ltar(0.0, -25, 0.0, 25);
	lppac1.SetLineColor(kRed);
	lppac2.SetLineColor(kRed);
	lppac3.SetLineColor(kRed);
	lppac1.SetLineWidth(2);
	lppac2.SetLineWidth(2);
	lppac3.SetLineWidth(2);
	ltar.SetLineWidth(2);

	long long total = tree->GetEntries();
	long long last_percentage = 0;
	printf("Processing   0%%");
	fflush(stdout);
	for (long long entry = 0; entry < total; ++entry) {
		if (entry * 100ll / total > last_percentage) {
			last_percentage = entry * 100ll / total;
			printf("\b\b\b\b%3lld%%", last_percentage);
			fflush(stdout);
		}
		tree->GetEntry(entry);
		beam_tree->GetEntry(entry);
		if (!beam.valid) continue;
		if (beam.tof < kTofMin || beam.tof > kTofMax) continue;

		double x[3] = {0.0, 0.0, 0.0};
		double y[3] = {0.0, 0.0, 0.0};
		bool valid[3] = {false, false, false};
		valid[0] = ComputePpacPosition(ppac, 0, 12, x[0], y[0]);
		valid[1] = ComputePpacPosition(ppac, 4, 13, x[1], y[1]);
		valid[2] = ComputePpacPosition(ppac, 8, 14, x[2], y[2]);

		if (valid[0]) hist_ppac1.Fill(x[0], y[0]);
		if (valid[1]) hist_ppac2.Fill(x[1], y[1]);
		if (valid[2]) hist_ppac3.Fill(x[2], y[2]);

		double x_intercept = 0.0;
		double x_slope = 0.0;
		if (LinearRegression(kPpacZ, x, valid, x_intercept, x_slope)) {
			FillLine(hist_xz, x_intercept, x_slope);
			double y_intercept = 0.0;
			double y_slope = 0.0;
			if (LinearRegression(kPpacZ, y, valid, y_intercept, y_slope)) {
				FillLine(hist_yz, y_intercept, y_slope);
				hist_target.Fill(x_intercept, y_intercept);
			}
		}
	}
	printf("\b\b\b\b100%%\n");

	canvas.cd(1);
	hist_ppac1.Draw("colz");
	canvas.cd(2);
	hist_ppac2.Draw("colz");
	canvas.cd(3);
	hist_ppac3.Draw("colz");
	canvas.cd(4);
	hist_target.Draw("colz");
	canvas.cd(5);
	hist_xz.Draw("colz");
	lppac1.Draw("same");
	lppac2.Draw("same");
	lppac3.Draw("same");
	ltar.Draw("same");
	canvas.cd(6);
	hist_yz.Draw("colz");
	lppac1.Draw("same");
	lppac2.Draw("same");
	lppac3.Draw("same");
	ltar.Draw("same");
	canvas.Update();
	app.Run();

	beam_file.Close();
	ppac_file.Close();
	return 0;
}
