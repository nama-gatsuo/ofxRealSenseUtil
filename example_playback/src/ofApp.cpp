#include "ofApp.h"

void ofApp::setup() {
	ofDisableArbTex();
	ofxRealSenseUtil::Settings s;
	s.colorRes = glm::vec2(1280, 720);
	s.depthRes = glm::vec2(1280, 720);
	s.useColor = false;
	s.useDepth = false;
	s.deviceId = -1;

	rs = std::make_shared<ofxRealSenseUtil::Player>(s, "20191124_145855.bag");
	rs->enableFlags(ofxRealSenseUtil::USE_DEPTH_MESH_POLYGON | ofxRealSenseUtil::USE_COLOR_TEXTURE);
	rs->start();

	panel.setup();
	panel.add(rs->getParameters());
}

void ofApp::update() {
	rs->update();
}

void ofApp::draw() {

	cam.begin();
	ofPushMatrix();
	ofScale(500.);
	rs->getColorImage().bind();
	rs->getPolygonMesh().draw();
	rs->getColorImage().unbind();
	ofPopMatrix();
	cam.end();

	ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate()), 10, 16);
	ofDrawBitmapString("key: [p] pause | [r] resume", 10, 32);
	panel.draw();
}

void ofApp::exit() {
	rs->stop();
}

void ofApp::keyPressed(int key) {
	if (key == 'p') {
		rs->pause();
	} else if (key == 'r') {
		rs->resume();
	}
}

