#ifndef __CRUSHER_H__
#define __CRUSHER_H__

#include <vector>
#include <memory>

#include "include/event/ore/decode_event.h"
#include "include/crush/raw_reader.h"

namespace forgerib {

class Crusher {
public:
	Crusher(
		const int num,
		const int run,
		const int crate,
		const std::vector<int> &module,
		const std::vector<int> &rate,
		const char *raw_path,
		const char *prefix
	);

	~Crusher() = default;

	DecodeEvent* GetEvent(bool report = true);

private:
	std::vector<std::unique_ptr<RawReader>> readers_;
	std::vector<DecodeEvent> events_;
	std::vector<bool> module_finish_;
	int finished_num_ = 0;
	size_t read_size_ = 0;
	size_t total_size_ = 0;
	size_t last_percentage_ = 0;
};

} // namespace forgerib

#endif
