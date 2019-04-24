#include "ofMain.h"
#include "ofApp.h"
int main() {
	ofGLFWWindowSettings s;
	s.setGLVersion(4, 5);
	s.setSize(1280, 840);
	ofCreateWindow(s);
	ofRunApp(new ofApp());
}
