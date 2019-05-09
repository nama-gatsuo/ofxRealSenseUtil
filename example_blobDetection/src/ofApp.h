#pragma once
#include "ofMain.h"
#include "ofxRealSenseUtil.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"
#include "ofxOsc.h"
#include "RectBorder.h"

class ofApp : public ofBaseApp{
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);

private:
	ofxRealSenseUtil::Interface rs;
	ofFbo binarized;
	ofFbo binarizedShrunk;
	ofShader binarizeShader;

	ofxCvContourFinder contourFinder;
	ofxCvGrayscaleImage binarizedImage;

	ofxPanel panel;
	ofParameter<float> zThres;
	ofParameterGroup oscGroup;
	ofParameter<bool> isDetectEnter;
	ofParameter<bool> isDetectLeave;
	ofxOscSender sender;
	
	RectBorder rect;
	std::vector<glm::vec2> pos;

	bool hasBlob;
};
