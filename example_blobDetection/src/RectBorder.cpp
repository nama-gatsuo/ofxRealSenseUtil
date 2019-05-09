#include "RectBorder.h"
#include "ofUtils.h"
#include "ofAppRunner.h"

RectBorder::RectBorder() : activeIndex(-1) {
	group.setName("border");
	group.add(pos.set(
		"position", glm::vec2(100.), glm::vec2(0.), glm::vec2(ofGetWidth(), ofGetHeight())
	));
	group.add(width.set("width", 100., 0., ofGetWidth()));
	group.add(height.set("height", 100., 0., ofGetHeight()));

	corners.assign(4, glm::vec2(0.));
	updateCorners();

	ofAddListener(ofEvents().mousePressed, this, &RectBorder::mousePressed);
	ofAddListener(ofEvents().mouseDragged, this, &RectBorder::mouseDragged);
	ofAddListener(ofEvents().mouseReleased, this, &RectBorder::mouseReleased);
}
RectBorder::~RectBorder() {
	ofRemoveListener(ofEvents().mousePressed, this, &RectBorder::mousePressed);
	ofRemoveListener(ofEvents().mouseDragged, this, &RectBorder::mouseDragged);
	ofRemoveListener(ofEvents().mouseReleased, this, &RectBorder::mouseReleased);
}

bool RectBorder::isInside(const glm::vec2& p) {
	return glm::all(glm::lessThan(
		glm::vec2(p - pos.get()), glm::vec2(width.get(), height.get())
	));
}
void RectBorder::draw() {
	updateCorners();

	ofPushStyle();

	ofSetColor(255);
	ofNoFill();
	ofDrawRectangle(pos, width, height);
	
	for (int i = 0; i < 4; i++) {
		if (i == activeIndex) {
			ofFill();
			ofSetColor(255);
			ofDrawRectangle(corners[i] - glm::vec2(6), 12, 12);
		} else {
			ofNoFill();
			ofSetColor(255, 128, 255);
			ofDrawRectangle(corners[i] - glm::vec2(5), 10, 10);
		}
	}

	ofPopStyle();
}
void RectBorder::mousePressed(ofMouseEventArgs& mouseArgs) {
	lastPos = glm::vec2(mouseArgs.x, mouseArgs.y);
	activeIndex = getNearIndex(lastPos);
}
void RectBorder::mouseDragged(ofMouseEventArgs& mouseArgs) {
	glm::vec2 current(mouseArgs.x, mouseArgs.y);
	glm::vec2 delta = current - lastPos;
	updateRect(current, delta);
	lastPos = current;
}
void RectBorder::mouseReleased(ofMouseEventArgs& mouseArgs) {
	glm::vec2 current(mouseArgs.x, mouseArgs.y);
	glm::vec2 delta = current - lastPos;
	updateRect(current, delta);
	lastPos = current;
	activeIndex = -1;
}
int RectBorder::getNearIndex(const glm::vec2& p, float threshold) {
	using namespace glm;
	for (int i = 0; i < corners.size(); i++) {
		float d = distance(vec2(p), vec2(corners[i]));
		if (d < threshold) {
			return i;
		}
	}
	return -1;
}
void RectBorder::updateRect(const glm::vec2& current, const glm::vec2& delta) {
	if (activeIndex >= 0) {

		corners[activeIndex] += delta;

		// update width
		if (activeIndex == 0 || activeIndex == 3) {
			width.set(corners[3].x - corners[0].x);
			if (activeIndex == 0) corners[3].y = corners[0].y;
			else corners[0].y = corners[3].y;
		} else {
			width.set(corners[2].x - corners[1].x);
			if (activeIndex == 2) corners[1].y = corners[2].y;
			else corners[2].y = corners[1].y;
		}

		// update height
		if (activeIndex == 0 || activeIndex == 1) {
			height.set(corners[1].y - corners[0].y);
			if (activeIndex == 0) corners[1].x = corners[0].x;
			else corners[0].x = corners[1].x;
		} else {
			height.set(corners[2].y - corners[3].y);
			if (activeIndex == 2) corners[3].x = corners[2].x;
			else corners[2].x = corners[3].x;
		}

		pos.set(corners[0]);

	} else if (isInside(current)) {

		for (auto& p : corners) {
			p += delta;
		}
		pos += delta;
	
	}

	updateCorners();

}
void RectBorder::updateCorners(){
	corners[0] = pos.get();
	corners[1] = pos.get() + glm::vec2(0., height.get());
	corners[2] = pos.get() + glm::vec2(width.get(), height.get());
	corners[3] = pos.get() + glm::vec2(width.get(), 0.); 
}