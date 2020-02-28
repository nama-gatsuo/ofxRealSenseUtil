#pragma once
#include "rsServer.h"

namespace ofxRealSenseUtil {

	class Player : public Server {
	public:
		Player() : Server("RealsensePlayer") {}
		Player(const std::string& name) : Server(name) {}

		void open(const std::string& filePath) {
			pipe = std::make_shared<rs2::pipeline>();
			// config = rs2::config();
			// Enable reading from file
			config.enable_device_from_file("data/" + filePath);
			bOpen = true;
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
