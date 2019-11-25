#include "ofApp.h"

void ofApp::setup() {

	rs.enableFlags(ofxRealSenseUtil::USE_DEPTH_TEXTURE | ofxRealSenseUtil::USE_COLOR_TEXTURE);
	rs.start();

	panel.setup();
	panel.add(rs.getParameters());

}

void ofApp::update() {
	rs.update();
}

void ofApp::draw() {
	
	rs.getColorImage().draw(0, 0, 640, 360);
	rs.getDepthImage().draw(0, 360, 640, 360);

	panel.draw();
	ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate()), 10, 20);
}

void ofApp::exit() {
	rs.stop();
}

void ofApp::keyPressed(int key) {
	if (key == ' ') {
		rs.startRecord("test.bag");
	} else if (key == 's') {
		rs.endRecord();
	}
}

