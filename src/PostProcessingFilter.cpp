#include "PostProcessingFilter.h"

using namespace ofxRealSenseUtil;

FilterOptions::FilterOptions(const std::string& name, ofPtr<rs2::filter> filter) :
	name(name), filter(filter)
{

	const std::array<rs2_option, 6> possible_filter_options = {
		RS2_OPTION_FILTER_MAGNITUDE,
		RS2_OPTION_FILTER_SMOOTH_ALPHA,
		RS2_OPTION_MIN_DISTANCE,
		RS2_OPTION_MAX_DISTANCE,
		RS2_OPTION_FILTER_SMOOTH_DELTA,
		RS2_OPTION_HOLES_FILL
	};

	group.setName(name);
	group.add(isEnabled.set(false));

	//Go over each filter option and create a slider for it
	for (rs2_option opt : possible_filter_options) {
		if (filter->supports(opt)) {
			rs2::option_range range = filter->get_option_range(opt);
			std::string key = filter->get_option_name(opt);

			if (checkInteger(range)) {
				group.add(iParams[opt].set(key, range.def, range.min, range.max));
			} else {
				group.add(fParams[opt].set(key, range.def, range.min, range.max));
			}

		}
	}
}

bool FilterOptions::checkInteger(const rs2::option_range& range) const {
	const auto is_integer = [](float f) {
		return (fabs(fmod(f, 1)) < std::numeric_limits<float>::min());
	};
	return is_integer(range.min) && is_integer(range.max) && is_integer(range.def) && is_integer(range.step);
}

PostProcessingFilters::PostProcessingFilters() {
	// Decimation - reduces depth frame density
	ofPtr<rs2::decimation_filter> dec_filter = std::make_shared<rs2::decimation_filter>();
	// Threshold  - removes values outside recommended range
	ofPtr<rs2::threshold_filter> thr_filter = std::make_shared<rs2::threshold_filter>();
	// Spatial    - edge-preserving spatial smoothing
	ofPtr<rs2::spatial_filter> spat_filter = std::make_shared<rs2::spatial_filter>();
	// Temporal   - reduces temporal noise
	ofPtr<rs2::temporal_filter> temp_filter = std::make_shared<rs2::temporal_filter>();
	// Declare disparity transform from depth to disparity and vice versa
	ofPtr<rs2::disparity_transform> depth_to_disparity = std::make_shared<rs2::disparity_transform>(true);
	dispalityToDepth = std::make_shared<rs2::disparity_transform>(false);

	filters.emplace_back("Decimate", dec_filter);
	filters.emplace_back("Threshold", thr_filter);
	filters.emplace_back("Disparity", depth_to_disparity);
	filters.emplace_back("Spatial", spat_filter);
	filters.emplace_back("Temporal", temp_filter);

	group.setName("Depth_filters");
	for (auto& filter : filters) {
		group.add(filter.getParameters());
	}
}

void PostProcessingFilters::filter(rs2::frame& depthFrame) {
	if (!depthFrame) return;

	bool revert_disparity = false;
	for (auto filter : filters) {

		if (filter.isEnabled) {
			for (const auto& pair : filter.fParams) {
				filter.filter->set_option(pair.first, pair.second);
			}
			for (const auto& pair : filter.iParams) {
				filter.filter->set_option(pair.first, pair.second);
			}
			if (filter.name == "Disparity") {
				revert_disparity = true;
			}
			depthFrame = filter.filter->process(depthFrame);
		}
	}
	if (revert_disparity) {
		depthFrame = dispalityToDepth->process(depthFrame);
	}
}