#include "ofApp.h"

void ofApp::setup() {
	ofDisableArbTex();

	// A recorded file should be located in bin/data
	// You can record via Realsense View or ofxRealSenseUtil::Recorder
	rs = std::make_shared<ofxRealSenseUtil::Player>("20191124_145855.bag");
	rs->enableFlags(ofxRealSenseUtil::USE_DEPTH_MESH_POLYGON | ofxRealSenseUtil::USE_COLOR_TEXTURE);
	rs->start();

	panel.setup();
	panel.add(rs->getParameters());
	panel.add(currentPosition.set("current", 0.f, 0.f, 1.f));
	panel.add(startPosition.set("startPos", 0.f, 0.f, 1.f));
	panel.add(isPlaying.set("isPlaying", true));

	isPlaying.addListener(this, &ofApp::onToggle);
	startPosition.addListener(this, &ofApp::onStartPosChanged);
}

void ofApp::update() {
	rs->update();
	currentPosition.set(rs->getProgress());
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

void ofApp::onToggle(bool&) {
	if (isPlaying) rs->resume();
	else rs->pause();
}

void ofApp::onStartPosChanged(float&) {
	rs->seek(startPosition.get());
}

