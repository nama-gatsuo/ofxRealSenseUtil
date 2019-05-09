#pragma once
#include "ofEvents.h"
#include "ofVectorMath.h"
#include "ofGraphics.h"
#include "ofParameter.h"

class RectBorder {
public:
	RectBorder();
	~RectBorder();

	void draw();
	ofRectangle getRect() {
		return ofRectangle(pos.get(), width.get(), height.get());
	}
	ofParameterGroup& getParameters() { return group; }
	
	void mousePressed(ofMouseEventArgs& mouseArgs);
	void mouseDragged(ofMouseEventArgs& mouseArgs);
	void mouseReleased(ofMouseEventArgs& mouseArgs);
private:
	bool isInside(const glm::vec2& p);
	int getNearIndex(const glm::vec2& p, float threshold = 10.);
	void updateRect(const glm::vec2& current, const glm::vec2& delta);
	void updateCorners();

	ofParameterGroup group;
	ofParameter<glm::vec2> pos;
	ofParameter<float> width;
	ofParameter<float> height;

	std::vector<glm::vec2> corners;
	glm::vec2 lastPos;
	int activeIndex;
};