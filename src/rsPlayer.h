#pragma once
#include "rsServer.h"

namespace ofxRealSenseUtil {

	class Player : public Server {
	public:
		Player() : Server("RealsensePlayer") {
			playerGroup.setName("PlayerParams");
			playerGroup.add(isPause.set("pause", false));
			playerGroup.add(seekValue.set("seek", 0.f, 0.f, 1.f));
			playerGroup.add(progress.set("progress", 0.f, 0.f, 1.f));
			seekValue.addListener(this, &Player::onSeek);
			isPause.addListener(this, &Player::onToggle);
			rsParams.add(playerGroup);
		}
		Player(const std::string& name) : Server(name) {
			seekValue.removeListener(this, &Player::onSeek);
		}

		void update() override {
			Server::update();
			progress = getProgress();
		}

		void open(const std::string& filePath) {
			pipe = std::make_shared<rs2::pipeline>();
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

		// Normalized value
		double getProgress() const {
			return (1. * getPosition()) / getDuration();
		}

		// Getters in nonoseconds
		int64_t getDuration() const {
			return device.as<rs2::playback>().get_duration().count();
		}
		uint64_t getPosition() const {
			return device.as<rs2::playback>().get_position();
		}

		void seek(float percent) {
			seekValue = percent;
		}

		// Event callbacks
		void onSeek(float& percent) {
			auto& playback = device.as<rs2::playback>();
			if (playback.current_status() != RS2_PLAYBACK_STATUS_STOPPED) {
				auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(playback.get_duration());
				duration.count();
				auto seekTime = std::chrono::duration<double, std::nano>(percent * duration.count());
				playback.seek(std::chrono::duration_cast<std::chrono::nanoseconds>(seekTime));
			}
		}

		void onToggle(bool&) {
			if (isPause) pause();
			else resume();
		}

	private:
		ofParameterGroup playerGroup;
		ofParameter<bool> isPause;
		ofParameter<float> seekValue;
		ofParameter<float> startPos;
		ofParameter<float> progress;
	};
};