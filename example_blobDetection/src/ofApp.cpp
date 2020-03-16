#include "ofApp.h"

void ofApp::setup() {

	ofSetFrameRate(30);
	ofBackground(0);

	// Accessing parameters in ofxRealSenseUtil via ofParameterGroup
	auto& rsParams = rs.getParameters();
	auto& dParams = rsParams[rsParams.getPosition("depthMeshParams")].castGroup();
	dParams[dParams.getPosition("useColorTexture")].cast<bool>() = true;
	dParams[dParams.getPosition("useDepthTexture")].cast<bool>() = true;
	dParams[dParams.getPosition("usePointCloud")].cast<bool>() = false;
	dParams[dParams.getPosition("usePolygonMesh")].cast<bool>() = false;

	rs.open();
	rs.start();

	binarizeShader.load("shader/binarize");
	binarized.allocate(1280, 720, GL_R8);
	binarizedShrunk.allocate(640, 360, GL_R8);
	
	panel.setup();
	panel.add(zThres.set("zThres", 1., 0.1, 5.));
	panel.add(state.getParameters());
	panel.add(rs.getParameters());
	panel.add(rect.getParameters());

	panel.setPosition(640 + 20, 360 + 80);
	panel.minimizeAll();
	panel.loadFromFile("settings.xml");

}

void ofApp::update() {
	rs.update();

	// create binarized image with threshold
	binarized.begin();
	ofClear(0);
	{
		binarizeShader.begin();
		binarizeShader.setUniform1f("zThres", zThres.get());
		binarizeShader.setUniformTexture("input", rs.getDepthTex(), 0);
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
	contourFinder.findContours(binarizedImage, 120, (640 * 360) / 4, 8, false);
	
	bool hasBlob = contourFinder.nBlobs > 0;
	state.update(hasBlob);
}

void ofApp::draw() {
	
	rs.getDepthTex().draw(0, 0, 640, 360);
	ofDrawBitmapStringHighlight("raw depth", 2, 16);
	
	binarizedImage.draw(640, 0, 640, 360);
	for (auto& blob : contourFinder.blobs) {
		blob.draw(640, 0);
	}
	ofDrawBitmapStringHighlight("binarized image", 642, 16);

	rs.getColorTex().draw(0, 360, 640, 360);
	ofDrawBitmapStringHighlight("raw color", 2, 376);

	ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate()), 640 + 20, 360 + 20);
	if (state.get()) {
		ofDrawBitmapStringHighlight("Detecting blob", 640 + 20, 360 + 40, ofColor(128, 128, 255));
	}

	rect.draw();
	panel.draw();
}

void ofApp::keyPressed(int key) {
	
}