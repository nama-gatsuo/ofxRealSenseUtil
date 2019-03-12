#pragma once
#include <librealsense2/rs.hpp>
#include "ofVboMesh.h"
#include "ofImage.h"
#include "ofThread.h"
#include "ofThreadChannel.h"
#include <map>

namespace ofxRealSenseUtil {

	class Interface : public ofThread {
	public:
		enum MeshMode { POLYGON, POINTCLOUD };

		Interface() : isNewFrame(true) {
			payload = { 10, POLYGON, 2. };

			startThread();
			pipe.start();

		}

		~Interface() {
			request.close();
			complete.close();
			waitForThread(true);
		}

		void update() {
			ofLogNotice() << "pay: " << payload.depthLimit;
			request.send(payload);

			isNewFrame = false;

			while (complete.tryReceive(fd)) {
				isNewFrame = true;
			}

			if (isNewFrame) {
				image.setFromPixels(fd.pixels);
				mesh = fd.mesh;
			}

		}

		const ofImage& getVideo() const {
			return image;
		}

		const ofVboMesh& getPointCloud() const {
			return mesh;
		}

		void setDepthLimit(float d) {
			payload.depthLimit = d;

		}

		void setDepthRes(int p) {
			payload.pixelSize = p;
		}


	private:
		void threadedFunction() override {
			RequestPayload pay;
			while (request.receive(pay)) {
				pay = payload;

				FrameData newFd;
				auto& frames = pipe.wait_for_frames();
				auto& depth = frames.get_depth_frame();
				auto& video = frames.get_color_frame();
				auto& points = pc.calculate(depth);

				switch (pay.mode) {
				case POLYGON:
					createMesh(newFd.mesh, points, pay.depthLimit, pay.pixelSize);
					break;
				case POINTCLOUD:
					createPointCloud(newFd.mesh, points, pay.depthLimit, pay.pixelSize);
					break;
				default:
					break;
				}


				newFd.pixels.setFromPixels(
					(unsigned char *)video.get_data(),
					video.get_width(), video.get_height(), OF_IMAGE_COLOR
				);

				complete.send(std::move(newFd));
			}

		}

		void createPointCloud(ofMesh& mesh, const rs2::points& ps, float depthLimit, int pixelSize) {

			mesh.clear();

			const rs2::vertex * vs = ps.get_vertices();
			int pNum = ps.size();

			const int w = 1280;
			const int h = 720;

			for (int y = 0; y < h - pixelSize; y += pixelSize) {
				for (int x = 0; x < w - pixelSize; x += pixelSize) {
					int i = y * w + x;
					const auto& v = vs[i];
					if (isinf(v.z) || v.z > depthLimit || v.z < 0.1) continue;
					mesh.addVertex(glm::vec3(v.x, -v.y, -v.z));
				}
			}

		}

		void createMesh(ofMesh& mesh, const rs2::points& ps, float depthLimit, int pixelSize) {

			mesh.clear();
			mesh.setMode(OF_PRIMITIVE_TRIANGLES);

			const rs2::vertex * vs = ps.get_vertices();

			const int w = 1280;
			const int h = 720;

			// list of index of depth map(x-y) - vNum
			std::map<int, int> vMap;
			std::vector<int> iList;
			std::vector<glm::vec3> vList;

			int indexCount = -1;
			for (int y = 0; y < h - pixelSize; y += pixelSize) {
				for (int x = 0; x < w - pixelSize; x += pixelSize) {

					int index[4] = {
						y * w + x,
						y * w + (x + pixelSize),
						(y + pixelSize) * w + x,
						(y + pixelSize) * w + (x + pixelSize)
					};
					glm::vec3 pos[4];
					int eraseCount = 0;
					bool eraseFlag[4]{ false, false, false, false };

					for (int i = 0; i < 4; i++) {
						const auto& v = vs[index[i]];
						pos[i] = glm::vec3(v.x, -v.y, -v.z);
						if (isinf(v.z) || v.z > depthLimit || v.z < 0.1) {
							eraseFlag[i] = true;
							eraseCount++;
						}
					}

					// try to check if possible to make square
					if (eraseCount >= 2) continue;
					else if (eraseCount == 1) {
						for (int i = 0; i < 4; i++) {
							if (!eraseFlag[i]) {
								// avoid double count
								if (vMap.count(index[i]) == 0) {
									vMap[index[i]] = ++indexCount;
									vList.push_back(pos[i]);
								}
								iList.push_back(vMap[index[i]]);
							}
						}
					} else if (eraseCount == 0) {
						for (int i = 0; i < 4; i++) {
							if (vMap.count(index[i]) == 0) {
								vMap[index[i]] = ++indexCount;
								vList.push_back(pos[i]);
							}
						}
						iList.push_back(vMap[index[0]]);
						iList.push_back(vMap[index[1]]);
						iList.push_back(vMap[index[2]]);

						iList.push_back(vMap[index[2]]);
						iList.push_back(vMap[index[1]]);
						iList.push_back(vMap[index[3]]);
					}
				}
			}

			for (auto& v : vList) {
				mesh.addVertex(v);
				mesh.addColor(ofFloatColor(v.x, v.y, v.z));
			}
			//ofLogNotice() << "v: " << mesh.getNumVertices();
			for (int vi : iList) {
				mesh.addIndex(vi);
			}
			//ofLogNotice() << "i: " << mesh.getNumIndices();
		}

		struct FrameData {
			ofMesh mesh;
			ofPixels pixels;
		} fd;

		struct RequestPayload {
			int pixelSize;
			MeshMode mode;
			float depthLimit;
		} payload;

		rs2::pipeline pipe;
		rs2::pointcloud pc;

		ofVboMesh mesh;
		ofImage image;
		bool isNewFrame;

		ofThreadChannel<RequestPayload> request;
		ofThreadChannel<FrameData> complete;
	};
}
