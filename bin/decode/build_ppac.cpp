#include <iostream>

#include <TFile.h>
#include <TTree.h>

#include "include/decode/builder.h"

struct PpacEvent {
	int flag;
	double time[15];
	int energy[15];
	bool cv[15];
};



int main(int argc, char **argv) {
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " [run].\n";
		return 1;
	}
	const int run = atoi(argv[1]);

	TString filename = TString::Format(
		"/data/disk1/test/build/ppac_%04d.root",
		run
	);
	TFile opf(filename, "recreate");
	TTree opt("tree", "ppac");
	PpacEvent ppac;
	opt.Branch("flag", &ppac.flag, "flag/I");
	opt.Branch("time", ppac.time, "t[15]/D");
	opt.Branch("energy", ppac.energy, "e[15]/I");
	opt.Branch("cv", ppac.cv, "cv[15]/O");
	// opt.Branch("ppac0_flag", &ppac[0].flag, "p0f/I");
	// opt.Branch("ppac0_x1", &ppac[0].x1, "p0x1/D");
	// opt.Branch("ppac0_x2", &ppac[0].x2, "p0x2/D");
	// opt.Branch("ppac0_y1", &ppac[0].y1, "p0y1/D");
	// opt.Branch("ppac0_y2", &ppac[0].y2, "p0y2/D");
	// opt.Branch("ppac0_cathode", &ppac[0].cathode, "p0c/D");
	// opt.Branch("ppac0_x1e", &ppac[0].x1e, "p0x1e/I");
	// opt.Branch("ppac0_x2e", &ppac[0].x2e, "p0x2e/D");
	// opt.Branch("ppac0_y1e", &ppac[0].y1e, "p0y1e/D");
	// opt.Branch("ppac0_y2e", &ppac[0].y2e, "p0y2e/D");
	// opt.Branch("ppac0_cathode_energy", &ppac[0].ce, "p0ce/D");
	// opt.Branch("ppac0_x1v", &ppac[0].x1v, "p0x1v/O");
	// opt.Branch("ppac0_x2v", &ppac[0].x2v, "p0x2v/O");
	// opt.BranTString ch("ppac0_y1v", &ppac[0].y1v, "p0y1v/O");
	// opt.Branch("ppac0_y2v", &ppac[0].y2v, "p0y2v/O");
	// opt.Branch("ppac0_cv", &ppac[0].cv, "p0cv/O");

	// opt.Branch("ppac1_flag", &ppac[1].flag, "p1f/I");
	// opt.Branch("ppac1_x1", &ppac[1].x1, "p1x1/D");
	// opt.Branch("ppac1_x2", &ppac[1].x2, "p1x2/D");
	// opt.Branch("ppac1_y1", &ppac[1].y1, "p1y1/D");
	// opt.Branch("ppac1_y2", &ppac[1].y2, "p1y2/D");
	// opt.Branch("ppac1_cathode", &ppac[1].cathode, "p1c/D");
	// opt.Branch("ppac1_x1e", &ppac[1].x1e, "p1x1e/I");
	// opt.Branch("ppac1_x2e", &ppac[1].x2e, "p1x2e/D");
	// opt.Branch("ppac1_y1e", &ppac[1].y1e, "p1y1e/D");
	// opt.Branch("ppac1_y2e", &ppac[1].y2e, "p1y2e/D");
	// opt.Branch("ppac1_cathode_energy", &ppac[1].ce, "p1ce/D");
	// opt.Branch("ppac1_x1v", &ppac[1].x1v, "p1x1v/O");
	// opt.Branch("ppac1_x2v", &ppac[1].x2v, "p1x2v/O");
	// opt.Branch("ppac1_y1v", &ppac[1].y1v, "p1y1v/O");
	// opt.Branch("ppac1_y2v", &ppac[1].y2v, "p1y2v/O");
	// opt.Branch("ppac1_cv", &ppac[1].cv, "p1cv/O");

	// opt.Branch("ppac2_flag", &ppac[2].flag, "p2f/I");
	// opt.Branch("ppac2_x1", &ppac[2].x1, "p2x1/D");
	// opt.Branch("ppac2_x2", &ppac[2].x2, "p2x2/D");
	// opt.Branch("ppac2_y1", &ppac[2].y1, "p2y1/D");
	// opt.Branch("ppac2_y2", &ppac[2].y2, "p2y2/D");
	// opt.Branch("ppac2_cathode", &ppac[2].cathode, "p2c/D");
	// opt.Branch("ppac2_x1e", &ppac[2].x1e, "p2x1e/I");
	// opt.Branch("ppac2_x2e", &ppac[2].x2e, "p2x2e/D");
	// opt.Branch("ppac2_y1e", &ppac[2].y1e, "p2y1e/D");
	// opt.Branch("ppac2_y2e", &ppac[2].y2e, "p2y2e/D");
	// opt.Branch("ppac2_cathode_energy", &ppac[2].ce, "p0ce/D");
	// opt.Branch("ppac2_x1v", &ppac[2].x1v, "p2x1v/O");
	// opt.Branch("ppac2_x2v", &ppac[2].x2v, "p2x2v/O");
	// opt.Branch("ppac2_y1v", &ppac[2].y1v, "p2y1v/O");
	// opt.Branch("ppac2_y2v", &ppac[2].y2v, "p2y2v/O");
	// opt.Branch("ppac2_cv", &ppac[2].cv, "p2cv/O");

	std::vector<int> module{0};
	std::vector<int> rate{500};
	glimmer::Builder builder(1, run, module, rate, "/data/xia2/test/raw", "data");

	printf("Building PPAC   0%%");
	fflush(stdout);

	std::vector<glimmer::DecodeEvent> events;
	while (builder.GetEvent(events, 1000, true) > 0) {
		ppac.flag = 0;
		for (const glimmer::DecodeEvent &event : events) {
			if ((ppac.flag & (1 << event.channel)) == 0) {
				ppac.flag |= 1 << event.channel;
				ppac.time[event.channel] = event.time;
				ppac.energy[event.channel] = event.energy;
				ppac.cv[event.channel] = event.cfd_valid;
			} else if (event.energy > ppac.energy[event.channel]) {
				ppac.time[event.channel] = event.time;
				ppac.energy[event.channel] = event.channel;
				ppac.cv[event.channel] = event.cfd_valid;
			}
		}
		opt.Fill();
	}
	printf("\b\b\b\b100%%\n");

	opf.cd();
	opt.Write();
	opf.Close();
	return 0;
}
