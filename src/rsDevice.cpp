#include "rsDevice.h"

namespace ofxRealSenseUtil {
	void Device::open() {
		open(Settings{
			glm::ivec2(1280, 720),
			glm::ivec2(1280, 720),
			true,
			true,
			0,
			true
		});
	}
	void Device::open(const Settings& s) {
		defaultSettings = s;

		refreshConfig(defaultSettings);

		bOpen = true;
	}
	void Device::startRecord(const std::string& path) {
		if (!bOpen) {
			ofLogError(__FUNCTION__) << "Not opened yet.";
			return;
		}

		if (!device.as<rs2::recorder>()) {
			stop();
			refreshConfig(defaultSettings);
			config.enable_record_to_file("data/" + path);// Enabling to record to file
			start();
			ofLogNotice(__FUNCTION__) << "Start recording";
		} else {
			ofLogWarning(__FUNCTION__) << "Already started.";
		}
	}
	void Device::endRecord() {
		if (device.as<rs2::recorder>()) {
			stop();
			refreshConfig(defaultSettings);
			start();
			ofLogNotice(__FUNCTION__) << "End recording";
		} else {
			ofLogWarning(__FUNCTION__) << "not started yet.";
		}
	}
	void Device::refreshConfig(const Settings& s) {
		config = rs2::config();
		pipe = std::make_shared<rs2::pipeline>();

		rs2::context ctx;
		rs2::device_list deviceList = ctx.query_devices();

		if (deviceList.size() == 0) {
			//throw std::runtime_error("No device detected. Is it plugged in?");
			ofLogWarning(__FUNCTION__) << "No device detected. Is realsense plugged in?";
		} else if (deviceList.size() <= s.deviceId) {
			ofLogWarning(__FUNCTION__) << "Device id (" << s.deviceId << ") is not available.";
		} else {
			device = deviceList[s.deviceId];
			const std::string& deviceSerial = device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
			config.enable_device(deviceSerial);
			ofLogNotice(__FUNCTION__) << deviceSerial << " is active!";

			if (!s.autoExposure) {
				setOption(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 0);
			}

		}

		if (s.useDepthSensor) config.enable_stream(RS2_STREAM_DEPTH, s.depthRes.x, s.depthRes.y, RS2_FORMAT_Z16, 30);
		if (s.useColorSensor) config.enable_stream(RS2_STREAM_COLOR, s.colorRes.x, s.colorRes.y, RS2_FORMAT_RGB8, 30);
		
	}
	
}