#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	ofGLFWWindowSettings s;
	s.setGLVersion(4, 4);
	s.setSize(1280, 720);
	ofCreateWindow(s);
	ofRunApp(new ofApp());
}
