#ifndef __RAW_READER_H__
#define __RAW_READER_H__

#include <fstream>

#include <TString.h>

#include "include/event/raw/raw_event.h"
#include "include/event/raw/decode_event.h"

namespace glimmer {

class RawReader {
public:

	RawReader(
		const char* raw_path,
		const char *data_name,
		const int run,
		const int module,
		const int rate
	);

	~RawReader();

	int Read(
		DecodeEvent *event,
		RawEnergySum **sum = nullptr,
		RawQDC **qdc = nullptr,
		RawExternalTime **ext = nullptr,
		DecodeTraces *trace = nullptr
	);

	size_t FileSize() const;

private:
	// run information
	// raw data path
	std::string raw_path_;
	// raw data file name prefix
	std::string data_name_;
	// run number
	int run_;
	// module index, start from 0
	int module_;
	// rate
	int rate_;
	// file handler
	std::ifstream handle_;
	// header for reading
	RawHeader header_;
	// body for reading
	unsigned int body_[14];
	// traces for reading
	DecodeTraces traces_;

	inline TString RawFileName() const {
		return TString::Format(
			"%s/%04d/%s_R%04d_M%02d.bin",
			raw_path_.c_str(), run_, data_name_.c_str(), run_, module_
		);
	}
};

}

#endif // __RAW_READER_H__