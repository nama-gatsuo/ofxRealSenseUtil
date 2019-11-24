#include "ofApp.h"

void ofApp::setup() {

	clipShader.load("shader/shader");
	ofSetVerticalSync(false);

	deferred.init();

	auto bg = deferred.createPass<ofxDeferred::BgPass>();
	bg->begin();
	ofClear(239, 244, 255, 255);
	bg->end();

	auto e = deferred.createPass<ofxDeferred::EdgePass>();
	e->setEdgeColor(ofFloatColor(0.6));
	e->setUseReadColor(true);

	auto s = deferred.createPass<ofxDeferred::SsaoPass>();
	s->setOcculusionRadius(5.);
	s->setDarkness(.5);

	sh = deferred.createPass<ofxDeferred::ShadowLightPass>();
	sh->setPosition(glm::vec3(300, 500, - 400));
	sh->lookAt(glm::vec3(0));
	sh->setAmbientColor(ofFloatColor(0.86, 0.86, 0.83));
	sh->setDiffuseColor(ofFloatColor(0.95));
	sh->setDarkness(.6);

	auto d = deferred.createPass<ofxDeferred::DofPass>();
	d->setEndPointsCoC(glm::vec2(0.05, .3));
	d->setFoculRange(glm::vec2(0.04, .3));

	cam.setNearClip(0.);
	cam.setFarClip(3000.);

	rs = std::make_shared<ofxRealSenseUtil::Interface>();
	rs->enableFlags(ofxRealSenseUtil::USE_DEPTH_MESH_POLYGON | ofxRealSenseUtil::USE_COLOR_TEXTURE);
	rs->setClipRect(ofRectangle(glm::vec2((1280 - 480) / 2, (720 - 480) / 2), 480, 480));

	panel.setup();
	panel.add(rs->getParameters());
	panel.add(deferred.getParameters());
	panel.add(offsetY.set("offsetY", 0., -3.f, 3.f));
	panel.add(clipZ.set("clipZ", 0., 0.f, 6.f));
}

void ofApp::update() {
	rs->update();
}

void ofApp::draw() {
	
	auto drawFunc = [&](float lds, bool isShadow) {
		
		glEnable(GL_CLIP_DISTANCE0);
		glEnable(GL_CLIP_DISTANCE1);
		glEnable(GL_CLIP_DISTANCE2);
		
		int numAngle = 6;
		float t = PI / float(numAngle);

		clipShader.begin();
		clipShader.setUniform1f("lds", lds);
		clipShader.setUniform1f("isShadow", isShadow ? 1 : 0);
		//clipShader.setUniform4f("clipPlane0", glm::vec4(cos(t), sin(t), 0, 0));
		clipShader.setUniform4f("clipPlane0", glm::vec4(1, 0, 0, 0));
		clipShader.setUniform4f("clipPlane1", glm::vec4(-cos(t), sin(t), 0, 0));
		clipShader.setUniform4f("clipPlane2", glm::vec4(0, 0, 1, 0));
		clipShader.setUniform1f("offsetY", offsetY);
		clipShader.setUniform1f("clipZ", clipZ);
		clipShader.setUniformTexture("tex", rs->getColorImage(), 0);

;		ofPushMatrix();
		ofScale(500.);
		for (int i = 0; i < 6; i++) {
			ofPushMatrix();
			ofRotateZ(60.f * i);
			for (int k = 0; k < 2; k++) {
				ofScale(1, 1, -1);
				for (int j = 0; j < 2; j++) {
					ofScale(1, -1, 1);
					rs->getPolygonMesh().draw();
				}
			}
			ofPopMatrix();
		}
		
		ofPopMatrix();

		clipShader.end();

		glDisable(GL_CLIP_DISTANCE0);
		glDisable(GL_CLIP_DISTANCE1);
		glDisable(GL_CLIP_DISTANCE2);
	};


	sh->beginShadowMap(cam, true);
	drawFunc(sh->getLinearScalar(), true);
	sh->endShadowMap();

	deferred.begin(cam, true);
	drawFunc(1. / (cam.getFarClip() - cam.getNearClip()), false);
	deferred.end();

	//sh->debugDraw();
	panel.draw();
	//ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate()), 10, 20);
}

void ofApp::exit() {
	rs->stopThread();
}

