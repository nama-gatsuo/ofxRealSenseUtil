#pragma once
#include "rsServer.h"

namespace ofxRealSenseUtil {


	class Device : public Server {
	public:

		struct Settings {
			glm::ivec2 depthRes;
			glm::ivec2 colorRes;
			bool useColorSensor;
			bool useDepthSensor;
			int deviceId;
			bool autoExposure;
		};

		Device() : Server("RealsenseDevice") {}
		Device(const std::string& name) : Server(name) {}

		void open();
		void open(const Settings& s);

		void startRecord(const std::string& path);
		void endRecord();

		void pauseRecord() {
			device.as<rs2::recorder>().pause();
		}
		void resumeRecord() {
			device.as<rs2::recorder>().resume();
		}

		// There're so many options like exosure, white balance and so on.
		// Refer rs2_option enum and rs2::options class.
		void setOption(rs2_option opt, float value) {
			device.first<rs2::depth_sensor>().set_option(opt, value);
		}

	private:
		Settings defaultSettings;

		void refreshConfig(const Settings& s);

	};

	using DeviceSettings = Device::Settings;
}