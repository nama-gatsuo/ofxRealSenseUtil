#pragma once
#include "ofParameter.h"
#include "ofUtils.h"
#include "ofxOsc.h"
class StateWrapper {
public:
	StateWrapper() : binaryState(false), timeStamp(0.) {
		group.setName("osc");
		group.add(isDetectEnter.set("send_on_enter", true));
		group.add(isDetectLeave.set("send_on_leave", true));
		group.add(ignoringJitterDuration.set("ignore_duration", .3, 0., 4.));
		sender.setup("127.0.0.1", 5555);
	}
	void update(bool hasBlob) {
		if (hasBlob != binaryState) {
			float t = ofGetElapsedTimef() - timeStamp;
			if (t > ignoringJitterDuration.get()) {
				binaryState = hasBlob;
				sendOsc(binaryState);
				timeStamp = ofGetElapsedTimef();
			} else {
				ofLogNotice("StateWrapper") << "ignore blob detection";
			}
		}
	}
	bool get() { return binaryState; }
	ofParameterGroup& getParameters() { return group; }
private:
	void sendOsc(bool enter) {
		std::string address("/sensor/");
		if (enter) {
			address += "enter";
			ofLogNotice("StateWrapper") << "Someone entered!";
		} else {
			address += "leave";
			ofLogNotice("StateWrapper") << "Someone left!";
		}
		ofxOscMessage msg;
		msg.setAddress(address);
		sender.sendMessage(msg);

	}

	bool binaryState; // on or off
	float timeStamp;

	ofParameterGroup group;
	ofParameter<bool> isDetectEnter;
	ofParameter<bool> isDetectLeave;
	ofParameter<float> ignoringJitterDuration;
	ofxOscSender sender;

};
