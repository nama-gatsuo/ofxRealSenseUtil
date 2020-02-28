#include "ofApp.h"

void ofApp::setup() {
	ofxRealSenseUtil::DeviceSettings s;
	s.depthRes = glm::vec2(640, 480);
	s.colorRes = glm::vec2(640, 480);
	s.deviceId = 0;
	s.useColor = true;
	s.useDepth = true;
	s.createFlags = ofxRealSenseUtil::USE_TEXTURE_DEPTH | ofxRealSenseUtil::USE_TEXTURE_COLOR;
	s.autoExposure = true;

	rs.open(s);
	rs.start();

	isRecording = false;

}

void ofApp::update() {
	rs.update();
}

void ofApp::draw() {
	if (rs.getColorTex().isAllocated()) {
		rs.getColorTex().draw(0, 0);
		rs.getDepthTex().draw(640, 0);
	}
	
	ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate()), 10, 16);
	ofDrawBitmapString("key: [r] start recoring | [s] stop recorgind", 10, 32);
	if (isRecording) ofDrawBitmapStringHighlight("Recording...", 10, 48, ofColor(255, 0, 0));
}

void ofApp::exit() {
	rs.stop();
}

void ofApp::keyPressed(int key) {
	if (key == 'r') {
		isRecording = true;
		rs.startRecord(ofGetTimestampString() + ".bag");
	} else if (key == 's') {
		isRecording = false;
		rs.endRecord();
	}
}

