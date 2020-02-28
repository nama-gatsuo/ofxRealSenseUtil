#pragma once
#include "ofMain.h"
#include "ofxRealSenseUtil.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"
#include "RectBorder.h"
#include "StateWrapper.h"

class ofApp : public ofBaseApp{
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);

private:
	ofxRealSenseUtil::Device rs;
	ofFbo binarized;
	ofFbo binarizedShrunk;
	ofShader binarizeShader;

	ofxCvContourFinder contourFinder;
	ofxCvGrayscaleImage binarizedImage;

	ofxPanel panel;
	ofParameter<float> zThres;
	
	RectBorder rect;
	std::vector<glm::vec2> pos;

	StateWrapper state;
};
