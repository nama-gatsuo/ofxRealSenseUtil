#pragma once

#include "ofMain.h"
#include "ofxRealSenseUtil.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void exit();

	void onToggle(bool&);
	void onStartPosChanged(float&);
private:
	ofPtr<ofxRealSenseUtil::Player> rs;
	ofxPanel panel;
	ofParameter<bool> isPlaying;
	ofParameter<float> currentPosition;
	ofParameter<float> startPosition;
	ofEasyCam cam;
};