#ifndef __COLOSSUS_H__
#define __COLOSSUS_H__

#include <vector>
#include <iostream>

#include <TFile.h>
#include <TTree.h>

namespace glimmer {

int ForgeVme(
	const char *trigger_path,
	const char *vme_path,
	const char *output_path,
	const long long range,
	const long long window
);

};

#endif	// __COLOSSUS_H__