#pragma once

#include <filesystem>
#include <string>

namespace forgerib {

inline std::string JoinPath(const std::string &base, const std::string &path) {
	if (path.empty()) return base;
	const std::filesystem::path other(path);
	if (other.is_absolute()) return other.string();
	return (std::filesystem::path(base) / other).string();
}

} // namespace forgerib
