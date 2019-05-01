#pragma once
#include "ofMain.h"

class RectBoarder {
public:
	RectBoarder() : activeIndex(-1) {
		pos = { glm::ivec2(0., 0.), glm::ivec2(0., 100.), glm::ivec2(100., 100.), glm::ivec2(100., 0) };

		ofAddListener(ofEvents().mousePressed, this, &RectBoarder::mousePressed);
		ofAddListener(ofEvents().mouseDragged, this, &RectBoarder::mouseDragged);
		ofAddListener(ofEvents().mouseReleased, this, &RectBoarder::mouseReleased);
	}
	~RectBoarder() {
		ofRemoveListener(ofEvents().mousePressed, this, &RectBoarder::mousePressed);
		ofRemoveListener(ofEvents().mouseDragged, this, &RectBoarder::mouseDragged);
		ofRemoveListener(ofEvents().mouseReleased, this, &RectBoarder::mouseReleased);
	}

	bool isInside(const glm::vec2& p) {
		return glm::all(glm::lessThan(
			glm::vec2(p - rect.getPosition()),
			glm::vec2(rect.getWidth(), rect.getHeight())
		));
	}
	void draw() {
		
		ofPushStyle();

		ofSetColor(255);
		ofDrawLine(pos[0], pos[1]);
		ofDrawLine(pos[1], pos[2]);
		ofDrawLine(pos[2], pos[3]);
		ofDrawLine(pos[3], pos[0]);
		
		ofSetColor(255, 0, 255);
		if (activeIndex > 0) {
			ofDrawRectangle(pos[activeIndex] - glm::ivec2(6), 12, 12);
		}

		ofSetColor(255, 128, 255);
		for (auto& p : pos) {
			ofDrawRectangle(p - glm::ivec2(5), 10, 10);
		}
		
		ofPopStyle();
	}

	ofRectangle& getRect() {
		return rect;
	}

	void mousePressed(ofMouseEventArgs& mouseArgs) {
		lastPos = glm::ivec2(mouseArgs.x, mouseArgs.y);
		activeIndex = getNearIndex(lastPos);
	}

	void mouseDragged(ofMouseEventArgs& mouseArgs) {
		glm::ivec2 current(mouseArgs.x, mouseArgs.y);
		glm::ivec2 delta = current - lastPos;
		if (activeIndex >= 0) {
			pos[activeIndex] += delta;
		} else if (isInside(current)) {
			for (auto& p : pos) {
				p += delta;
			}
			rect.position += glm::vec3(delta, 0.);
		}
		updateRect();
		lastPos = current;
	}

	void mouseReleased(ofMouseEventArgs& mouseArgs) {
		glm::ivec2 current(mouseArgs.x, mouseArgs.y);
		glm::ivec2 delta = current - lastPos;
		if (activeIndex >= 0) {
			pos[activeIndex] += delta;
		} else if (isInside(current)) {
			for (auto& p : pos) {
				p += delta;
			}
			rect.position += glm::vec3(delta, 0.);
		}
		updateRect();
		activeIndex = -1;
		lastPos = current;
	}

	int getNearIndex(const glm::ivec2& p, float threshold = 10.) {
		for (int i = 0; i < pos.size(); i++) {
			float d = glm::distance(glm::vec2(p), glm::vec2(pos[i]));
			if (d < threshold) {
				return i;
			}
		}
		return -1;
	}

	glm::ivec2& operator[](int i) { return pos[i]; }

	void updateRect() {
		// update width
		if (activeIndex == 0 || activeIndex == 3) {
			rect.setWidth(pos[3].x - pos[0].x);
			if (activeIndex == 0) pos[3].y = pos[0].y;
			else pos[0].y = pos[3].y;
		} else {
			rect.setWidth(pos[2].x - pos[1].x);
			if (activeIndex == 2) pos[1].y = pos[2].y;
			else pos[2].y = pos[1].y;
		}

		// update height
		if (activeIndex == 0 || activeIndex == 1) {
			rect.setHeight(pos[1].y - pos[0].y);
			if (activeIndex == 0) pos[1].x = pos[0].x;
			else pos[0].x = pos[1].x;
		} else {
			rect.setHeight(pos[2].y - pos[3].y);
			if (activeIndex == 2) pos[3].x = pos[2].x;
			else pos[2].x = pos[3].x;
		}
		rect.setPosition(glm::vec3(pos[0], 0.));
	}

private:
	ofRectangle rect;
	std::vector<glm::ivec2> pos;
	glm::ivec2 lastPos;
	int activeIndex;
};