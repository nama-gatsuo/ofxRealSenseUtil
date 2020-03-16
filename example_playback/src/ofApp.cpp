#include "ofApp.h"

void ofApp::setup() {
	ofDisableArbTex();
	ofSetFrameRate(60);

	// Accessing parameters in ofxRealSenseUtil via ofParameterGroup
	auto& rsParams = rs.getParameters();
	auto& dParams = rsParams[rsParams.getPosition("depthMeshParams")].castGroup();
	dParams[dParams.getPosition("useColorTexture")].cast<bool>() = true;
	dParams[dParams.getPosition("usePolygonMesh")].cast<bool>() = true;

	// A recorded file should be located in bin/data
	// You can record via Realsense Viewer or ofxRealSenseUtil::Recorder		
	rs.open("20191124_145855.bag");
	rs.start();

	panel.setup();
	panel.add(rsParams);
}

void ofApp::update() {
	rs.update();
}

void ofApp::draw() {

	cam.begin();
	ofPushMatrix();
	ofScale(500.f);

	if (rs.getColorTex().isAllocated()) {
		rs.getColorTex().bind();
		rs.getPolygonMesh().draw();
		rs.getColorTex().unbind();
	}
	ofPopMatrix();
	cam.end();

	ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate()), 10, 16);
	ofDrawBitmapString("key: [p] pause | [r] resume", 10, 32);
	panel.draw();
}

void ofApp::exit() {
	rs.stop();
}

void ofApp::keyPressed(int key) {
	if (key == 'r') {
		isPlaying = true;
	} else if (key == 's') {
		isPlaying = false;
	}
}

