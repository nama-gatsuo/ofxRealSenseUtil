#pragma once
#include <librealsense2/rs.hpp>
#include <map>
#include "ofTypes.h"
#include "ofParameter.h"

namespace ofxRealSenseUtil {
	class FilterOptions {
	public:
		FilterOptions(const std::string& name, ofPtr<rs2::filter> filter);

		bool checkInteger(const rs2::option_range& range) const;
		ofParameterGroup& getParameters() {
			return group;
		}

		std::string name;
		ofParameter<bool> isEnabled;
		std::map<rs2_option, ofParameter<int>> iParams;
		std::map<rs2_option, ofParameter<float>> fParams;
		ofPtr<rs2::filter> filter;

	private:
		ofParameterGroup group;
	};
	
	class PostProcessingFilters {
	public:
		PostProcessingFilters();

		void filter(rs2::frame& depthFrame);
		
		ofParameterGroup& getParameters() {
			return group;
		}

	private:
		std::vector<FilterOptions> filters;
		ofPtr<rs2::disparity_transform> dispalityToDepth;
		ofParameterGroup group;
	};

}