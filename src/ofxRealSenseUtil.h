#pragma once
#include "RealSenseServer.h"

namespace ofxRealSenseUtil {

	
	
	class Grabber : public Server {
	public:
		Grabber() : Grabber({ rsDepthRes, rsColorRes, true, true, 0 }) {}
		Grabber(const Settings& s) : Server("realsense_grabber") {
			refreshConfig(s);
		}
	};

	class Recorder : public Server {
	public:
		Recorder() : Recorder({ rsDepthRes, rsColorRes, true, true, 0 }) {}
		Recorder(const Settings& s) : Server("realsense_recorder") {
			refreshConfig(s);
			defaultSettings = s;
		}

		void startRecord(const std::string& path) {
			if (!device.as<rs2::recorder>()) {
				stop();
				refreshConfig(defaultSettings);
				config.enable_record_to_file("data/" + path);// Enabling to record to file
				start();
				ofLogNotice("ofxRealSenseUtil") << "Start recording";
			} else {
				ofLogWarning("ofxRealSenseUtil::Recorder") << "Already started.";
			}	
		}

		void endRecord() {
			if (device.as<rs2::recorder>()) {
				stop();
				refreshConfig(defaultSettings);
				start();
				ofLogNotice("ofxRealSenseUtil") << "End recording";
			} else {
				ofLogWarning("ofxRealSenseUtil::Recorder") << "not started yet.";
			}
		}

		void pause() {
			device.as<rs2::recorder>().pause();
		}
		void resume() {
			device.as<rs2::recorder>().resume();
		}
		
	private:
		Settings defaultSettings;
	};

	class Player : public Server {
	public:
		Player(const std::string& path) : Player({ rsDepthRes, rsColorRes, false, false, -1 }, path) {}
		Player(const Settings& s, const std::string& path) : Server("realsense_player_" + path) {
			refreshConfig(s);
			// Enable reading from file
			config.enable_device_from_file("data/" + path);
		}
		void pause() {
			device.as<rs2::playback>().pause();
		}
		void resume() {
			device.as<rs2::playback>().resume();
		}
		void seek(double percent) {
			auto& playback = device.as<rs2::playback>();
			if (playback.current_status() != RS2_PLAYBACK_STATUS_STOPPED) {
				auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(playback.get_duration());
				duration.count();
				auto seekTime = std::chrono::duration<double, std::nano>(percent * duration.count());
				playback.seek(std::chrono::duration_cast<std::chrono::nanoseconds>(seekTime));
			}
		}
		double getProgress() const {
			return (1. * getPosition()) / getDuration();
		}
		int64_t getDuration() const {
			return device.as<rs2::playback>().get_duration().count();
		}
		uint64_t getPosition() const {
			return device.as<rs2::playback>().get_position();
		}
	};
};
