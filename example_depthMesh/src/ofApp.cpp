#include "ofApp.h"

void ofApp::setup() {

	clipShader.load("shader/shader");
	//ofSetVerticalSync(false);

	deferred.init();
	deferred.createPass<ofxDeferred::BgPass>();
	deferred.createPass<ofxDeferred::EdgePass>();
	deferred.createPass<ofxDeferred::SsaoPass>();
	sh = deferred.createPass<ofxDeferred::ShadowLightPass>();
	deferred.createPass<ofxDeferred::DofPass>();
	
	cam.setNearClip(0.);
	cam.setFarClip(3000.);

	rs.open();
	rs.start();

	// Accessing parameters in ofxRealSenseUtil via ofParameterGroup
	auto& rsParams = rs.getParameters();
	auto& dParams = rsParams[rsParams.getPosition("depthMeshParams")].castGroup();
	dParams[dParams.getPosition("useColorTexture")].cast<bool>() = true;
	dParams[dParams.getPosition("usePolygonMesh")].cast<bool>() = true;

	panel.setup();
	panel.add(rsParams);
	panel.add(offsetY.set("offsetY", 0., -3.f, 3.f));
	panel.add(clipZ.set("clipZ", 0., 0.f, 6.f));
	panel.add(deferred.getParameters());
	panel.loadFromFile("settings.xml");
	panel.minimizeAll();
	
}

void ofApp::update() {
	rs.update();
}

void ofApp::draw() {
	
	int numAngle = 2;

	auto drawFunc = [&](float lds, bool isShadow) {
		glEnable(GL_CLIP_DISTANCE0);
		glEnable(GL_CLIP_DISTANCE1);
		glEnable(GL_CLIP_DISTANCE2);
		
		float t = PI / float(numAngle);

		clipShader.begin();
		clipShader.setUniform1f("lds", lds);
		clipShader.setUniform1f("isShadow", isShadow ? 1 : 0);
		clipShader.setUniform4f("clipPlane0", glm::vec4(1, 0, 0, 0));
		clipShader.setUniform4f("clipPlane1", glm::vec4(-cos(t), sin(t), 0, 0));
		clipShader.setUniform4f("clipPlane2", glm::vec4(0, 0, 1, 0));
		clipShader.setUniform1f("offsetY", offsetY);
		clipShader.setUniform1f("clipZ", clipZ);
		clipShader.setUniformTexture("tex", rs.getColorTex(), 0);

		ofPushMatrix();
		ofScale(500.);
		ofTranslate(0, 0, 1.);
		//rs.getPolygonMesh().draw();
		for (int i = 0; i < numAngle; i++) {
			ofPushMatrix();
			ofRotateZ(360.f / numAngle * i);
			for (int k = 0; k < 2; k++) {
				ofScale(1, 1, -1);
				for (int j = 0; j < 2; j++) {
					ofScale(1, -1, 1);
					rs.getPolygonMesh().draw();
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
	drawFunc(1.f / (cam.getFarClip() - cam.getNearClip()), false);
	deferred.end();

	//sh->debugDraw
	deferred.debugDraw();
	panel.draw();
	ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate()), 10, 20);
}

void ofApp::exit() {
	rs.stop();
}

