#include "include/decode/raw_reader.h"

#include <iostream>
#include <filesystem>

namespace glimmer {

RawReader::RawReader(
	const char* raw_path,
	const char* data_name,
	const int run,
	const int module,
	const int rate
)
: raw_path_(raw_path)
, data_name_(data_name)
, run_(run)
, module_(module)
, rate_(rate) {
	handle_.open(RawFileName(), std::ios::binary);
}

RawReader::~RawReader() {
	if (handle_.is_open()) handle_.close();
}

int RawReader::Read(
	DecodeEvent *event,
	RawEnergySum **sum,
	RawQDC **qdc,
	RawExternalTime **ext,
	DecodeTraces *trace
) {
	// if (!handle_.good()) {
	// 	std::cout << "Module " << module_ << " not good." << std::endl;
	// 	event->used = true;
	// 	return 0;
	// }

	// read header
	handle_.read((char*)&header_, sizeof(RawHeader));
	//if (header_.data[0] == 0) {
		//std::cout << "Found empty word.\n";
		//handle_.seekg(-12, std::ios::cur);
	//}
	std::streamsize bytes = handle_.gcount();
	if (bytes != 16) return bytes;

	// get event length and increase offset
	int event_length = (header_.data[0] >> 17) & 0x3fff;
	// fill information
	event->module = (unsigned short)(((header_.data[0] >> 4) & 0xf) - 2);
	event->channel = header_.data[0] & 0xf;
	event->energy = header_.data[3] & 0xffff;
	// cfd
	event->cfd = 0.0;
	if (rate_ == 100) {
		bool cfd_force = (header_.data[2] >> 31) != 0;
		if (!cfd_force) {
			event->cfd = double((header_.data[2] >> 16) & 0x7fff);
			event->cfd = event->cfd / 32768.0 * 10.0;
		}
		event->cfd_valid = !cfd_force;
	} else if (rate_ == 250) {
		bool cfd_force = (header_.data[2] >> 31) != 0;
		if (!cfd_force) {
			unsigned int cfds = (header_.data[2] >> 30) & 0x1;
			event->cfd = double((header_.data[2] >> 16) & 0x3fff);
			event->cfd = (event->cfd / 16384.0 - cfds) * 4.0;
		}
		event->cfd_valid = !cfd_force;
	} else { 					//500
		unsigned int cfds = (header_.data[2] >> 29) & 0x7;
		bool cfd_force = cfds == 7;
		if (!cfd_force) {
			event->cfd = double((header_.data[2] >> 16) & 0x1fff);
			event->cfd = (event->cfd / 8192.0 + cfds - 1) * 2.0;
		}
		event->cfd_valid = !cfd_force;
	}
	// timestamp
	event->timestamp = int64_t(header_.data[2] & 0xffff) << 32;
	event->timestamp |= header_.data[1];
	if (rate_ == 250) event->timestamp *= 8;
	else event->timestamp *= 10;
	// time
	event->time = double(event->timestamp) + event->cfd;
	// used
	event->used = false;

	int header_length = (header_.data[0] >> 12) & 0x1f;
	int trace_length = (header_.data[3] >> 16) & 0x7fff;
	if (event_length - header_length != trace_length/2) {
		std::cout << "header " << header_length << ", event " << event_length << ", trace " << trace_length << "\n";
		//header_length = event_length - trace_length/2;
		return 16;
	}

	// read body
	if (header_length > 4 && ((header_length-4)&1)==0 && ((header_length-4)>>4)==0) {
		handle_.read((char*)body_, (header_length-4)*4);
		const unsigned int *body_pos = body_;
		bytes += handle_.gcount();

		// check existence of energy sum
		if (sum) {
			if ((header_length - 4) & 0x4) {
				*sum = (RawEnergySum*)body_pos;
				body_pos += 4;
			} else {
				*sum = nullptr;
			}
		}

		// check existence of QDC
		if (qdc) {
			if ((header_length - 4) & 0x8) {
				// get QDC
				*qdc = (RawQDC*)body_pos;
				body_pos += 8;
			} else {
				*qdc = nullptr;
			}
		}

		// check existence of external timestamp
		if (ext) {
			if ((header_length - 4) & 0x2) {
				*ext = (RawExternalTime*)body_pos;
				body_pos += 2;
			} else {
				*ext = nullptr;
			}
		}
	} else {
		if (sum) *sum = nullptr;
		if (qdc) *qdc = nullptr;
		if (ext) *ext = nullptr;
	}

	// read traces
	int trace_out_of_range = header_.data[3] >> 31;
	if (trace) {
		trace->length = trace_length;
		trace->out_of_range = trace_out_of_range == 1;
		handle_.read((char*)(trace->samples), trace_length*2);
		bytes += handle_.gcount();
	} else {
		auto last_pos = handle_.tellg();
		handle_.seekg(trace_length*2, std::ios_base::cur);
		bytes += handle_.tellg() - last_pos;
	}

	if (bytes == event_length*4) return bytes;
	return -bytes;
}

size_t RawReader::FileSize() const {
	if (std::filesystem::exists(RawFileName().Data())) {
			return std::filesystem::file_size(RawFileName().Data());
	}
	return 0;
}

}
