#include "include/forge/alignment.h"

#include <iostream>
#include <iomanip>
#include <algorithm>

#include <TGraph.h>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TF1.h>

#include "include/event/forge/align_event.h"

namespace glimmer {

struct Edge {
	// left edge
	double left;
	// right edge
	double right;
};


Edge EdgeDetect(TH1F *hist, double threshold_factor) {
	// bins of histogram
	double sum = 0.0;
	int bins = hist->GetNbinsX();
	for (int i = 1; i <= bins; ++i) {
		sum += hist->GetBinContent(i);
	}
	double average = sum / bins;
	double threshold = average*threshold_factor;

	const int check_num = 4;
	// detect rise edge
	bool detect_rise = false;
	// detect fall edge
	bool detect_fall = false;
	// return result of left edge and right edge
	Edge result{0.0, 0.0};
	for (int i = 1; i <= bins-check_num+1; ++i) {
		int over_threshold = 0;
		for (int j = 0; j < check_num; ++j) {
			if (hist->GetBinContent(i+j) > threshold) {
				++over_threshold;
			}
		}
		if (!detect_rise) {
			if (over_threshold == check_num) {
				detect_rise = true;
				result.left = hist->GetBinCenter(i);
				result.right = result.left;
			}
		} else if (over_threshold == 0) {
			detect_fall = true;
			result.right = hist->GetBinCenter(i);
			break;
		}
	}
	if (!detect_fall) result.right = hist->GetBinCenter(bins-check_num+1);
	return result;
}


int Align(
	const std::vector<long long> &xia_times,
	const std::vector<long long> &xia_entries,
	const std::vector<long long> &vme_times,
	const std::vector<long long> &vme_entries,
	//const long long xia_total_entries,
	const char *output_path,
	int group_num,
	double search_window,
	double search_low_bound,
	double search_high_bound,
	bool report,
	bool verbose
) {
	// output file
	TFile opf(output_path, "recreate");
	// offset-group graph in first, second. third match
	TGraph offset_vs_group[3];
	// window-group graph, in first, second, third match
	TGraph window_vs_group[3];
	// output tree
	TTree opt("tree", "alignment result");
	AlignEvent event;
	SetupOutput(&opt, event);

	// first loop aligned events
	long long first_align_events = 0;
	// second loop aligned events
	long long second_align_events = 0;
	// third loop aligned events
	long long third_align_events = 0;
	// align events
	long long align_events = 0;
	// oversize events
	long long oversize_events = 0;

	// group size
	size_t group_size = vme_times.size() / group_num + 1;
	// lower bound of offset in each group, set general for the first group
	double lower_bound = search_low_bound;
	// upper bound of offset in each group, set general for the first group
	double upper_bound = search_high_bound;
	for (int group = 0; group < group_num; ++group) {
		if (report && verbose) {
			std::cout << "----------------------------------------\n"
				<< "group " << group << "\n";
		}

		// start VME event index of this group
		size_t group_start_index = group_size * group;
		// end VME event index of this group
		size_t group_end_index = group_size * (group + 1);
		group_end_index = group_end_index < vme_times.size()
			? group_end_index : vme_times.size();

		// maximum match rate in all offsets
		double max_match_rate = 0;
		// offset of max match rate
		double max_match_offset = 0;
		// graph of match rate VS offset
		TGraph match_rate_vs_offset;
		// loop to search the for the offset with fixed steps
		for (
			double offset = lower_bound;
			offset <= upper_bound;
			offset += search_window
		) {
			// number of matched events
			long long match_events = 0;
			// first time match to search for the max match rate
			for (
				size_t vme_index = group_start_index;
				vme_index < group_end_index;
				++vme_index
			) {
				// break the loop if reach the end of array
				if (vme_index >= vme_times.size()) break;
				// VME time
				long long vme_time = vme_times[vme_index];
				// loop XIA events to find match events
				for (
					auto xia_iter = std::lower_bound(
						xia_times.begin(),
						xia_times.end(),
						vme_time + offset
					);
					xia_iter != xia_times.end();
					++xia_iter
				) {
					if (*xia_iter > vme_time + offset + search_window) break;
					++match_events;
				}
			}
			// record match rate in graph
			double match_rate = double(match_events) / double(group_size);
			match_rate_vs_offset.AddPoint(offset, match_rate);
			if (max_match_rate < match_rate) {
				// std::cout << "Change offset to "<< offset
				// 	<< " with rate from " << max_match_rate
				// 	<< " to " << match_rate << "\n";
				max_match_rate = match_rate;
				max_match_offset = offset;
			}
			// show match rate of each seconds
			if (report && verbose && (long long)offset % 1'000'000'000 == 0) {
				std::cout << "  " << std::setprecision(10)
					<< offset / 1'000'000'000.0 << " s"
					<< "  max match rate " << max_match_rate
					<< "  offset " << max_match_offset << "\n";
			}
		}
		// save rate-offset graph
		match_rate_vs_offset.Write(TString::Format("rate%d", group));

		// current offset
		double offset = max_match_offset;
		// current window
		double window = search_window;
		// total number of match events
		long long match_events = 0;
		// oversize events
		long long oversize = 0;
		// write to graph
		offset_vs_group[0].AddPoint(group, offset);
		window_vs_group[0].AddPoint(group, window);

		// time window for first match
		TH1F first_match_window(
			TString::Format("ht%d_1", group), "window", 200, 0, window
		);
		// first match out of the loop to calculate the average and variance
		for (
			size_t vme_index = group_start_index;
			vme_index < group_end_index;
			++vme_index
		) {
			// break the loop if reach the end of array
			if (vme_index >= vme_times.size()) break;
			// VME time
			long long vme_time = vme_times[vme_index];
			// match count of current VME event
			int match_count = 0;
			// loop XIA events to find match events
			for (
				auto xia_iter = std::lower_bound(
					xia_times.begin(),
					xia_times.end(),
					vme_time + offset
				);
				xia_iter != xia_times.end();
				++xia_iter
			) {
				if (*xia_iter > vme_time + offset + window) break;
				double difference = double(*xia_iter - vme_time - offset);
				first_match_window.Fill(difference);
				++match_events;
				++match_count;
			}
			if (match_count == 1) {
				++first_align_events;
			} else {
				++oversize;
			}
		}
		// detect edge
		Edge edge = EdgeDetect(&first_match_window, 1.2);
		// save time window
		first_match_window.Write();
		// show the first match result
		if (report && verbose) {
			std::cout << "group " << group
				<< ", first match offset " << offset
				<< ", window " << window
				<< ", rate " << double(match_events) / group_size
				<< ", oversize " << oversize << "\n"
				<< "edge left " << edge.left
				<< ", edge right " << edge.right << "\n";
		}

		// the second match to adjust the offset and window
		// initialize
		offset += (edge.left + edge.right) / 2.0;
		window = (edge.right - edge.left) * 4.0;
		match_events = 0;
		oversize = 0;
		// write to graph
		offset_vs_group[1].AddPoint(group, offset);
		window_vs_group[1].AddPoint(group, window);
		// time window in the second match
		TH1F second_match_window(
			TString::Format("ht%d_2", group), "window", 50, -window, window
		);
		// the second match
		for (
			size_t vme_index = group_start_index;
			vme_index < group_end_index;
			++vme_index
		) {
			// break the loop if reach the end of array
			if (vme_index >= vme_times.size()) break;
			// VME time
			long long vme_time = vme_times[vme_index];
			// match count of current VME event
			int match_count = 0;
			// loop XIA events to find match events
			for (
				auto xia_iter = std::lower_bound(
					xia_times.begin(),
					xia_times.end(),
					vme_time + offset - window
				);
				xia_iter != xia_times.end();
				++xia_iter
			) {
				if (*xia_iter > vme_time + offset + window) break;
				double difference = double(*xia_iter - vme_time - offset);
				second_match_window.Fill(difference);
				++match_events;
				++match_count;
			}
			if (match_count == 1) {
				++second_align_events;
			} else {
				++oversize;
			}
		}
		// detect edge
		edge = EdgeDetect(&second_match_window, 1.8);
		// save time window
		second_match_window.Write();

		// show the second match result
		if (report && verbose) {
			std::cout
				<< "group " << group
				<< ", second match offset " << offset
				<< ", window " << window
				<< ", rate " << double(match_events) / group_size
				<< ", oversize " << oversize << "\n"
				<< "edge left " << edge.left
				<< ", edge right " << edge.right << "\n";
		}

		// the third match to fill the tree
		// initialize
		offset += (edge.left + edge.right) / 2.0;
		window = (edge.right - edge.left) / 1.6;
		offset -= window / 5.0;
		match_events = 0;
		oversize = 0;
		// write to graph
		offset_vs_group[2].AddPoint(group, offset);
		window_vs_group[2].AddPoint(group, window);
		// time window in the third match
		TH1F third_match_window(
			TString::Format("ht%d_3", group), "window", 30, -window, window
		);
		// the third match to fill the tree
		for (
			size_t vme_index = group_start_index;
			vme_index < group_end_index;
			++vme_index
		) {
			// break the loop if reach the end of array
			if (vme_index >= vme_times.size()) break;
			// VME time
			event.vme_time = vme_times[vme_index];
			event.vme_entry = vme_entries[vme_index];
			// match count of current VME event
			int match_count = 0;
			// initialize XIA time
			event.xia_time = -1.0;
			// loop XIA events to find match events
			for (
				auto xia_iter = std::lower_bound(
					xia_times.begin(),
					xia_times.end(),
					event.vme_time + offset - window
				);
				xia_iter != xia_times.end();
				++xia_iter
			) {
				if (*xia_iter > event.vme_time + offset + window) break;
				double difference = double(*xia_iter - event.vme_time - offset);
				third_match_window.Fill(difference);
				++match_events;
				++match_count;
				event.xia_time = *xia_iter;
				event.xia_entry = xia_entries[xia_iter - xia_times.begin()];
			}
			if (match_count == 1) {
				++third_align_events;
				++align_events;
			} else if (match_count > 1) {
				++oversize;
				++oversize_events;
			}
			if (event.xia_time > 0) opt.Fill();
		}
		// save time window
		third_match_window.Write();
		// show the third match result
		if (report && verbose) {
			std::cout
				<< "group " << group
				<< ", third match offset " << offset
				<< ", window " << window
				<< ", rate " << double(match_events) / group_size
				<< ", oversize " << oversize << "\n"
				<< "----------------------------------------\n";
		}

		// upate lower bound and upper bound
		lower_bound = max_match_offset - 2 * search_window;
		upper_bound = max_match_offset + 2 * search_window;
	}
	// save tree
	opf.cd();
	opt.Write();
	// save offsets and windows
	for (int i = 0; i < 3; ++i) {
		offset_vs_group[i].Write(TString::Format("goffset%d", i));
		window_vs_group[i].Write(TString::Format("gwindow%d", i));
	}
	// opf.WriteObject(
	// 	new TParameter<long long>("xia_total", xia_total_entries),
	// 	"xia_total_entries"
	// );
	opf.Close();

	if (report) {
		std::cout
			<< "First loop align rate "
			<< first_align_events << " / " << xia_times.size() << " "
			<< double(first_align_events) / double(xia_times.size()) << "\n"
			<< "Second loop align rate "
			<< second_align_events << " / " << xia_times.size() << " "
			<< double(second_align_events) / double(xia_times.size()) << "\n"
			<< "Third loop align rate "
			<< third_align_events << " / " << xia_times.size() << " "
			<< double(third_align_events) / double(xia_times.size()) << "\n"
			<< "XIA align rate "
			<< align_events << " / " << xia_times.size() << " "
			<< double(align_events) / double(xia_times.size()) << "\n"
			<< "VME align rate "
			<< align_events << " / " << vme_times.size() << " "
			<< double(align_events) / double(vme_times.size()) << "\n"
			<< "Oversize events "
			<< oversize_events << " / " << vme_times.size() << " "
			<< double(oversize_events) / double(vme_times.size()) << "\n";
	}
	return 0;
}


}
