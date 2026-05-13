#include "include/decode/decoder.h"

#include <iostream>

namespace glimmer {

Decoder::Decoder(
	const int num,
	const int run,
	const std::vector<int> &module,
	const std::vector<int> &rate,
	const char *raw_path,
	const char *prefix
) {
	total_size_ = 0;
	for (int i = 0; i < num; ++i) {
		readers_.push_back(std::make_unique<RawReader>(
			raw_path, prefix, run, module[i], rate[i]
		));
		events_.push_back(DecodeEvent());
		events_[i].used = true;
		total_size_ += readers_[i]->FileSize();
		module_finish_.push_back(false);
	}
	finished_num_ = 0;
	read_size_ = 0;
	last_percentage_ = 0;
}



DecodeEvent* Decoder::GetEvent(bool report) {
	if (report && read_size_ * 100 / total_size_ > last_percentage_) {
		last_percentage_ = read_size_ * 100 / total_size_;
		printf("\b\b\b\b%3lu%%", last_percentage_);
		fflush(stdout);
	}
	if (finished_num_ >= int(readers_.size())) return nullptr;

	while (finished_num_ < int(readers_.size())) {
		for (size_t m = 0; m < readers_.size(); ++m) {
			if (events_[m].used && !module_finish_[m]) {
				int bytes = readers_[m]->Read(events_.data()+m);
				if (bytes >= 16) {
					read_size_ += bytes;
				} else if (bytes == 0) {
					module_finish_[m] = true;
					finished_num_ += 1;
				}
			}
		}
		if (finished_num_ == int(readers_.size())) return nullptr;

		// find most early event
		double min_time = -1.0;
		size_t min_index = 100;
		for (size_t m = 0; m < readers_.size(); ++m) {
			if (events_[m].used) continue;
			if (min_time < 0 || events_[m].time < min_time) {
				min_time = events_[m].timestamp;
				min_index = m;
			}
		}
		events_[min_index].used = true;
		return events_.data()+min_index;
	}
	return nullptr;
}

}
