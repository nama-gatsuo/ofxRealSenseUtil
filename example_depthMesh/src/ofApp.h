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
	ofShader shader;
	ofPtr<ofxRealSenseUtil::Interface> rs;
	ofxDeferredProcessing deferred;
	ofxDeferred::ShadowLightPass::Ptr sh;
	
	bool tog;
	ofxPanel panel;
	ofParameter<float> lim;
	ofParameter<int> res;
};