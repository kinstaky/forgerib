#include "include/config.h"

#include <iostream>

#include "external/toml.hpp"

namespace forgerib {

namespace {

void LoadString(const toml::table &table, const char *key, std::string &value) {
	if (auto node = table[key].value<std::string>()) {
		value = *node;
	}
}

void LoadPaths(const toml::table &table, AppPaths &paths) {
	LoadString(table, "raw_xia0", paths.raw_xia0);
	LoadString(table, "raw_xia1", paths.raw_xia1);
	LoadString(table, "raw_xia2", paths.raw_xia2);
	LoadString(table, "raw_vme", paths.raw_vme);
	LoadString(table, "ore", paths.ore);
	LoadString(table, "grit", paths.grit);
	LoadString(table, "grain", paths.grain);
	LoadString(table, "ingot", paths.ingot);
}

void LoadPrefixes(const toml::table &table, RawPrefixes &prefix) {
	LoadString(table, "raw_xia0", prefix.raw_xia0);
	LoadString(table, "raw_xia1", prefix.raw_xia1);
	LoadString(table, "raw_xia2", prefix.raw_xia2);
	LoadString(table, "raw_vme", prefix.raw_vme);
}

} // namespace

int LoadConfig(const std::string &path, AppConfig &config) {
	try {
		config = AppConfig();
		toml::table table = toml::parse_file(path);
		if (auto node = table["workspace"].value<std::string>()) {
			config.workspace = *node;
		} else {
			std::cerr << "Error: Missing required key workspace in " << path << ".\n";
			return -1;
		}
		LoadString(table, "trigger", config.trigger);
		if (const auto *jump_run = table["jump_run"].as_array()) {
			for (size_t i = 0; i < jump_run->size(); ++i) {
				if (auto value = (*jump_run)[i].value<int>()) {
					config.jump_run.push_back(*value);
				}
			}
		}
		if (const auto *paths = table["paths"].as_table()) {
			LoadPaths(*paths, config.paths);
		}
		if (const auto *prefix = table["prefix"].as_table()) {
			LoadPrefixes(*prefix, config.prefix);
		}
	} catch (const toml::parse_error &err) {
		std::cerr << "Error: Parsing " << path << " failed:\n" << err << "\n";
		return -1;
	}
	return 0;
}

} // namespace forgerib
