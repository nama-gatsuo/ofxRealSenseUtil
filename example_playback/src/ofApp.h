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
	void keyPressed(int key);

private:
	ofxRealSenseUtil::Player rs;
	ofxPanel panel;
	ofParameter<bool> isPlaying;
	ofEasyCam cam;
};