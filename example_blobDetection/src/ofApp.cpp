#include "ofApp.h"

void ofApp::setup() {

	ofSetFrameRate(30);

	rs.enableFlags(ofxRealSenseUtil::USE_COLOR_TEXTURE | ofxRealSenseUtil::USE_DEPTH_TEXTURE);
	binarizeShader.load("shader/binarize");
	binarized.allocate(1280, 720, GL_R8);
	binarizedShrunk.allocate(640, 360, GL_R8);
	
	panel.setup();
	panel.add(zThres.set("zThres", 1., 0.1, 5.));
	panel.setPosition(640 + 20, 360 + 80);

	hasBlob = false;
}

void ofApp::update() {
	rs.update();

	binarized.begin();
	ofClear(0);
	{
		binarizeShader.begin();
		binarizeShader.setUniform1f("zThres", zThres.get());
		rs.getDepthImage().draw(0, 0);
		binarizeShader.end();
	}
	binarized.end();

	binarizedShrunk.begin();
	ofClear(0);
	binarized.draw(0, 0, 640, 360);
	binarizedShrunk.end();

	ofPixels pix;
	pix.setImageType(OF_IMAGE_GRAYSCALE);
	binarizedShrunk.readToPixels(pix);

	binarizedImage.setFromPixels(pix);

	contourFinder.findContours(binarizedImage, 80, (640 * 360) / 4, 8, false);
	
	if (contourFinder.nBlobs > 0) {
		if (!hasBlob) {
			ofLogNotice() << "Someone entered!";
		}
		hasBlob = true;
	} else {
		if (hasBlob) {
			ofLogNotice() << "Someone leaved!";
		}
		hasBlob = false;
	}
}

void ofApp::draw() {
	
	rs.getDepthImage().draw(0, 0, 640, 360);
	ofDrawBitmapStringHighlight("raw depth", 2, 16);
	
	binarizedImage.draw(640, 0, 640, 360);
	for (auto& blob : contourFinder.blobs) {
		blob.draw(640, 0);
	}
	ofDrawBitmapStringHighlight("binarized image", 642, 16);

	rs.getColorImage().draw(0, 360, 640, 480);
	ofDrawBitmapStringHighlight("raw color", 2, 376);

	ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate()), 640 + 20, 360 + 20);
	if (hasBlob) {
		ofDrawBitmapStringHighlight("Detecting blob", 640 + 20, 360 + 40);
	}

	panel.draw();
}

void ofApp::keyPressed(int key) {
	if (key == ' ') {
		
	}
}