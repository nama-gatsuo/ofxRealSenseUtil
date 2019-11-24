#pragma once

#include "ofMain.h"
#include "ofxRealSenseUtil.h"
#include "ofxDeferredShading.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void exit();

private:
	ofEasyCam cam;
	ofShader clipShader;
	ofPtr<ofxRealSenseUtil::Interface> rs;
	ofxDeferredProcessing deferred;
	ofxDeferred::ShadowLightPass::Ptr sh;
	
	bool tog;
	ofxPanel panel;
	ofParameter<float> offsetY;
	ofParameter<float> clipZ;
};