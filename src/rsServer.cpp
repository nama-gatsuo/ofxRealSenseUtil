#include "rsServer.h"

using namespace ofxRealSenseUtil;

Server::Server(const std::string& name) : bPlaying(false), bNewFrame(false) {
	rsParams.setName(name);
	rsParams.add(filters.getParameters());
	depthMeshParams.setName("depthMeshParams");
	depthMeshParams.add(useColorTexture.set("useColorTexture", true));
	depthMeshParams.add(useDepthTexture.set("useDepthTexture", false));
	depthMeshParams.add(usePointCloud.set("usePointCloud", false));
	depthMeshParams.add(usePolygonMesh.set("usePolygonMesh", true));
	depthMeshParams.add(depthPixelSize.set("pixelSize", 10, 1, 100));
	depthMeshParams.add(isClip.set("enableClip", false));
	depthMeshParams.add(p0.set("clip_p0", glm::vec2(0), glm::vec2(0), glm::vec2(640, 480)));
	depthMeshParams.add(p1.set("clip_p1", glm::vec2(1280, 720), glm::vec2(0), glm::vec2(1280, 720)));
	rsParams.add(depthMeshParams);
}

Server::~Server() {}

void Server::start() {
	if (!bOpen) {
		ofLogError(__FUNCTION__) << "Not opened yet.";
		return;
	}
	request = std::make_shared<ofThreadChannel<bool>>();
	response = std::make_shared<ofThreadChannel<FrameData>>();

	try {
		device = pipe->start(config).get_device();
		ofLogNotice(__FUNCTION__) << "start: " << device.get_info(RS2_CAMERA_INFO_NAME);
	} catch (const rs2::error& e) {
		ofLogError(__FUNCTION__) << e.what();
		return;
	}

	startThread();
	bPlaying = true;
}

void Server::stop() {
	bPlaying = false;
	request->close();
	response->close();
	
	waitForThread(true);
	
	pipe->stop();

	ofLogNotice(__FUNCTION__) << "stop: " << device.get_info(RS2_CAMERA_INFO_NAME);
}

void Server::update() {
	
	if (bPlaying) {
		bool r = true;
		request->send(r);
	}
	
	bNewFrame = false;

	while (response->tryReceive(fd)) {
		bNewFrame = true;
	}

	if (bNewFrame) {
		if (useColorTexture) {
			if (!colorTex.isAllocated()) {
				colorTex.allocate(fd.colorPix.getWidth(), fd.colorPix.getHeight(), GL_RGB8);
			}
			colorTex.loadData(fd.colorPix);
		} 
		if (useDepthTexture) {
			if (!depthTex.isAllocated() ||
				!depthTex.getWidth() != fd.depthPix.getWidth() || !depthTex.getHeight() != fd.depthPix.getHeight()) {
				depthTex.allocate(fd.depthPix.getWidth(), fd.depthPix.getHeight(), GL_RGB32F);
			}
			depthTex.loadData(fd.depthPix);
		}
		if (usePointCloud) {
			meshPointCloud.clear();
			meshPointCloud = std::move(fd.meshPointCloud);
		}
		if (usePolygonMesh) meshPolygon = std::move(fd.meshPolygon);
		
	}

}

void Server::threadedFunction() {
	bool r = true;
	while (request->receive(r)) {

		FrameData newFd;
		
		rs2::frameset frames;
		if (!pipe->poll_for_frames(&frames)) continue;
		
		auto& depth = frames.get_depth_frame();
		auto& color = frames.get_color_frame();

		pc.map_to(color);
		filters.filter(depth);

		if (useColorTexture) {
			newFd.colorPix.setFromPixels(
				(unsigned char*)color.get_data(),
				color.get_width(), color.get_height(), OF_IMAGE_COLOR
			);
		}

		glm::ivec2 depthRes(depth.get_width(), depth.get_height());

		const auto& points = pc.calculate(depth);


		if (useDepthTexture) {
			newFd.depthPix.setFromPixels(
				(float*)points.get_vertices(),
				depthRes.x, depthRes.y, OF_IMAGE_COLOR
			);
		}
		


		if (usePointCloud) {
			createPointCloud(newFd.meshPointCloud, points, depthRes, depthPixelSize.get());
		}
		if (usePolygonMesh) {
			createMesh(newFd.meshPolygon, points, depthRes, depthPixelSize.get());
		}

		response->send(std::move(newFd));
		
	}

}

void Server::createPointCloud(ofMesh& mesh, const rs2::points& ps, const glm::ivec2& res, int pixelSize) {

	if (!ps) return;

	mesh.clear();
	
	const rs2::vertex * vs = ps.get_vertices();
	const rs2::texture_coordinate * texCoords = ps.get_texture_coordinates();

	glm::ivec2 start(0, 0), end(res);
	if (isClip) {
		start = glm::max(glm::ivec2(p0.get()), glm::ivec2(0));
		end = glm::min(glm::ivec2(p1.get()), res);
	}
	
	for (int y = start.y + pixelSize; y < end.y; y += pixelSize) {
		for (int x = start.x + pixelSize; x < end.x; x += pixelSize) {
			int i = y * res.x + x;
			const auto& v = vs[i];
			const auto& uv = texCoords[i];

			if (!v.z) continue;
			mesh.addVertex(glm::vec3(v.x, -v.y, -v.z));
			mesh.addTexCoord(glm::vec2(uv.u, uv.v));
		}
	}

}

void Server::createMesh(ofMesh& mesh, const rs2::points& ps, const glm::ivec2& res, int pixelSize) {

	if (!ps) return;
	
	mesh.clear();
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);

	const rs2::vertex * vs = ps.get_vertices();
	const rs2::texture_coordinate * texCoords = ps.get_texture_coordinates();

	glm::ivec2 start(0, 0), end(res);
	if (isClip) {
		start = glm::max(glm::ivec2(p0.get()), glm::ivec2(0));
		end = glm::min(glm::ivec2(p1.get()), res);
	}
	const glm::ivec2 numTexel = glm::abs(start - end) / pixelSize;

	// list of index of depth map(x-y) - vNum
	std::unordered_map<int, int> vMap;

	int indexCount = -1;
	for (int y = start.y + pixelSize; y < end.y - pixelSize; y += pixelSize) {
		for (int x = start.x + pixelSize; x < end.x - pixelSize; x += pixelSize) {

			int index[4] = {
				y * res.x + x,
				y * res.x + (x + pixelSize),
				(y + pixelSize) * res.x + x,
				(y + pixelSize) * res.x + (x + pixelSize)
			};
			glm::vec3 pos[4];
			glm::vec2 uv[4];
			int eraseCount = 0;
			bool eraseFlag[4]{ false, false, false, false };

			for (int i = 0; i < 4; i++) {
				const auto& v = vs[index[i]];
				const auto& t = texCoords[index[i]];
				if (v.z) {
					pos[i] = glm::vec3(v.x, -v.y, -v.z);
					uv[i] = glm::vec2(t.u, t.v);
				} else {
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
							mesh.addVertex(pos[i]);
							mesh.addTexCoord(uv[i]);
						}
						mesh.addIndex(vMap[index[i]]);
					}
				}
			}
			else if (eraseCount == 0) {
				for (int i = 0; i < 4; i++) {
					if (vMap.count(index[i]) == 0) {
						vMap[index[i]] = ++indexCount;
						mesh.addVertex(pos[i]);
						mesh.addTexCoord(uv[i]);
					}
				}
				mesh.addIndex(vMap[index[0]]);
				mesh.addIndex(vMap[index[1]]);
				mesh.addIndex(vMap[index[2]]);

				mesh.addIndex(vMap[index[2]]);
				mesh.addIndex(vMap[index[1]]);
				mesh.addIndex(vMap[index[3]]);
			}
		}
	}
	float endTime = ofGetElapsedTimef();
}

const ofTexture& Server::getColorTex() const {
	if (!useColorTexture) {
		ofLogError(__FUNCTION__) << "Target flag is disabled!";
	}
	return colorTex;
}
const ofTexture& Server::getDepthTex() const {
	if (!useDepthTexture) {
		ofLogError(__FUNCTION__) << "Target flag is disabled!";
	}
	return depthTex;
}
const ofVboMesh& Server::getPointCloud() const {
	if (!usePointCloud) {
		ofLogError(__FUNCTION__) << "Target flag is disabled!";
	}
	return meshPointCloud;
}
const ofVboMesh& Server::getPolygonMesh() const {
	if (!usePolygonMesh) {
		ofLogError(__FUNCTION__) << "Target flag is disabled!";
	}
	return meshPolygon;
}