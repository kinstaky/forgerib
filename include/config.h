#pragma once

#include <string>
#include <vector>

namespace forgerib {

struct AppPaths {
	std::string raw_xia0 = "/data/raw/xia0";
	std::string raw_xia1 = "/data/raw/xia1";
	std::string raw_xia2 = "/data/raw/xia2";
	std::string raw_vme = "/data/raw/vme";
	std::string ore = "ore";
	std::string grit = "grit";
	std::string grain = "grain";
	std::string ingot = "ingot";
};

struct RawPrefixes {
	std::string raw_xia0 = "data";
	std::string raw_xia1 = "data";
	std::string raw_xia2 = "data";
	std::string raw_vme = "data";
};

struct AppConfig {
	std::string workspace = "/data";
	std::string trigger = "t1";
	std::vector<int> jump_run;
	AppPaths paths;
	RawPrefixes prefix;
};

int LoadConfig(const std::string &path, AppConfig &config);

} // namespace forgerib
