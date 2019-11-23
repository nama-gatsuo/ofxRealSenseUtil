#include "RealSenseServer.h"

namespace ofxRealSenseUtil {

	struct Settings {
		glm::ivec2 depthRes;
		glm::ivec2 colorRes;
		bool useColor;
		bool useDepth;
		int deviceId;
	};
	
	class Grabber : public Server {
	public:
		Grabber() : Grabber({ rsDepthRes, rsColorRes, true, true, 0 }) {}
		Grabber(const Settings& s) {
			
			auto source = std::make_shared<Source>();
			rs2::context ctx;
			auto& list = ctx.query_devices(); // Get a snapshot of currently connected devices

			if (list.size() == 0) {
				//throw std::runtime_error("No device detected. Is it plugged in?");
				ofLogWarning("ofxRealSenseUtil") << "No device detected. Is realsense plugged in?";
			} else if (list.size() <= s.deviceId) {
				ofLogWarning("ofxRealSenseUtil") << "Device id(" << s.deviceId << ") is not available.";
			} else {
				source->device = list[s.deviceId];
				const std::string& deviceSerial = source->device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
				
				source->config.enable_device(deviceSerial);
				if (s.useDepth) source->config.enable_stream(RS2_STREAM_DEPTH, s.depthRes.x, s.depthRes.y, RS2_FORMAT_Z16, 30);
				if (s.useColor) source->config.enable_stream(RS2_STREAM_COLOR, s.colorRes.x, s.colorRes.y, RS2_FORMAT_RGB8, 30);
				ofLogWarning("ofxRealSenseUtil") << deviceSerial << " is active!";
			}

			Server::source = source;
		}
	};

	class Recorder : public Server {
	public:
		Recorder(const std::string& path) : Recorder({ rsDepthRes, rsColorRes, true, true, 0 }, path) {}
		Recorder(const Settings& s, const std::string& path) {
			auto source = std::make_shared<Source>();
			rs2::context ctx;
			auto& list = ctx.query_devices(); // Get a snapshot of currently connected devices

			if (list.size() == 0) {
				//throw std::runtime_error("No device detected. Is it plugged in?");
				ofLogWarning("ofxRealSenseUtil") << "No device detected. Is realsense plugged in?";
			} else if (list.size() <= s.deviceId) {
				ofLogWarning("ofxRealSenseUtil") << "Device id(" << s.deviceId << ") is not available.";
			} else {
				source->device = list[s.deviceId];
				const std::string& deviceSerial = source->device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);

				source->config.enable_device(deviceSerial);
				if (s.useDepth) source->config.enable_stream(RS2_STREAM_DEPTH, s.depthRes.x, s.depthRes.y, RS2_FORMAT_Z16, 30);
				if (s.useColor) source->config.enable_stream(RS2_STREAM_COLOR, s.colorRes.x, s.colorRes.y, RS2_FORMAT_RGB8, 30);
				ofLogWarning("ofxRealSenseUtil") << deviceSerial << " is active!";

				// Enabling to record to file
				source->config.enable_record_to_file("data/" + path);
			}
			
			Server::source = source;
		}

		void pause() {
			source->device.as<rs2::recorder>().pause();
		}
		void resume() {
			source->device.as<rs2::recorder>().resume();
		}

	};

	class Player : public Server {
	public:
		Player(const std::string& path) : Player({ rsDepthRes, rsColorRes, true, true, 0 }, path) {}
		Player(const Settings& s, const std::string& path) {
			auto source = std::make_shared<Source>();
			// Enable reading from file
			source->config.enable_device_from_file("data/" + path);
			source->pipe.start(source->config);
			source->device = source->pipe.get_active_profile().get_device();
			Server::source = source;
		}
		void pause() {
			waitForThread(true);
			source->device.as<rs2::playback>().pause();
		}
		void resume() {
			waitForThread(true);
			source->device.as<rs2::playback>().resume();
		}
		void seek(float percent) {
			waitForThread(true);
			auto& playback = source->device.as<rs2::playback>();
			if (playback.current_status() != RS2_PLAYBACK_STATUS_STOPPED) {
				auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(playback.get_duration());
				duration.count();
				auto seekTime = std::chrono::duration<double, std::milli>(percent * duration.count());
				playback.seek(std::chrono::duration_cast<std::chrono::milliseconds>(seekTime));
			}
		}
		double getProgress() {
			auto& playback = source->device.as<rs2::playback>();
			int count = playback.get_duration().count(); // default unit is nanoseconds
			int currentPoistion = playback.get_position();
			return (1. * currentPoistion) / count;
		}
	};
};
