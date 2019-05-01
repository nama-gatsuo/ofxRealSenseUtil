#include "ofApp.h"

void ofApp::setup() {

	ofSetFrameRate(30);
	ofBackground(0);

	rs.enableFlags(ofxRealSenseUtil::USE_COLOR_TEXTURE | ofxRealSenseUtil::USE_DEPTH_TEXTURE);
	binarizeShader.load("shader/binarize");
	binarized.allocate(1280, 720, GL_R8);
	binarizedShrunk.allocate(640, 360, GL_R8);
	
	panel.setup();
	panel.add(zThres.set("zThres", 1., 0.1, 5.));
	panel.setPosition(640 + 20, 360 + 80);

	oscGroup.setName("osc");
	oscGroup.add(isDetectEnter.set("send_on_enter", true));
	oscGroup.add(isDetectLeave.set("send_on_leave", true));
	panel.add(oscGroup);
	panel.add(rs.getParameters());
	panel.minimizeAll();
	panel.loadFromFile("settings.xml");

	hasBlob = false;
}

void ofApp::update() {
	rs.update();

	// create binarized image with threshold
	binarized.begin();
	ofClear(0);
	{
		binarizeShader.begin();
		binarizeShader.setUniform1f("zThres", zThres.get());
		binarizeShader.setUniformTexture("input", rs.getDepthImage().getTexture(), 0);
		ofDrawRectangle(rect.getRect());
		binarizeShader.end();
	}
	binarized.end();

	// shrink the resolution of image
	binarizedShrunk.begin();
	ofClear(0);
	binarized.draw(0, 0, 640, 360);
	binarizedShrunk.end();

	// convert ofFbo to cvImage
	ofPixels pix;
	pix.setImageType(OF_IMAGE_GRAYSCALE);
	binarizedShrunk.readToPixels(pix);
	binarizedImage.setFromPixels(pix);

	// find blobs from cvImage
	contourFinder.findContours(binarizedImage, 80, (640 * 360) / 4, 8, false);
	
	if (contourFinder.nBlobs > 0) {
		if (!hasBlob) {
			ofLogNotice() << "Someone entered!";
			if (isDetectEnter) {
				ofxOscMessage msg;
				msg.setAddress("/sensor/enter");
				sender.sendMessage(msg);
			}
		}
		hasBlob = true;
	} else {
		if (hasBlob) {
			ofLogNotice() << "Someone leaved!";
			if (isDetectLeave) {
				ofxOscMessage msg;
				msg.setAddress("/sensor/leave");
				sender.sendMessage(msg);
			}
		}
		hasBlob = false;
	}
	
	sender.setup("127.0.0.1", 5555);
}

void ofApp::draw() {
	
	rs.getDepthImage().draw(0, 0, 640, 360);
	ofDrawBitmapStringHighlight("raw depth", 2, 16);
	
	binarizedImage.draw(640, 0, 640, 360);
	for (auto& blob : contourFinder.blobs) {
		blob.draw(640, 0);
	}
	ofDrawBitmapStringHighlight("binarized image", 642, 16);

	rs.getColorImage().draw(0, 360, 640, 360);
	ofDrawBitmapStringHighlight("raw color", 2, 376);

	ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate()), 640 + 20, 360 + 20);
	if (hasBlob) {
		ofDrawBitmapStringHighlight("Detecting blob", 640 + 20, 360 + 40, ofColor(128, 128, 255));
	}

	rect.draw();

	panel.draw();
}

void ofApp::keyPressed(int key) {
	if (key == ' ') {
		
	}
}