#include "ofxRealSenseUtil.h"

using namespace ofxRealSenseUtil;

Interface::Interface() : isNewFrame(true), flags(0) {
	payload.flags = flags;

	rs2::context ctx;
	auto& list = ctx.query_devices(); // Get a snapshot of currently connected devices
	if (list.size() == 0) {
		//throw std::runtime_error("No device detected. Is it plugged in?");
		ofLogWarning("ofxRealSenseUtil") << "No device detected. Is realsense plugged in?";
	} else {
		rs2::config cfg;
		// Use a configuration object to request only depth from the pipeline
		cfg.enable_stream(RS2_STREAM_DEPTH, rsDepthRes.x, rsDepthRes.y, RS2_FORMAT_Z16, 30);
		cfg.enable_stream(RS2_STREAM_COLOR, rsColorRes.x, rsColorRes.y, RS2_FORMAT_RGB8, 30);
		// Start streaming with the above configuration
		pipe.start(cfg);
		startThread();
	}

	rsParams.setName("ofxRealSenseUtil");
	rsParams.add(filters.getParameters());

	depthMeshParams.setName("depthMeshParams");
	depthMeshParams.add(depthPixelSize.set("pixelSize", 10, 1, 100));
	rsParams.add(depthMeshParams);

}

Interface::~Interface() {
	request.close();
	complete.close();
	waitForThread(true);
	pipe.stop();
}

void Interface::update() {

	request.send(payload);

	isNewFrame = false;

	while (complete.tryReceive(fd)) {
		isNewFrame = true;
	}

	if (isNewFrame) {
		if (checkFlags(USE_COLOR_TEXTURE)) colorImage.setFromPixels(fd.colorPix);
		if (checkFlags(USE_DEPTH_TEXTURE)) depthImage.setFromPixels(fd.depthPix);
		if (checkFlags(USE_DEPTH_MESH_POINTCLOUD) || checkFlags(USE_DEPTH_MESH_POINTCLOUD_COLOR)) {
			meshPointCloud = fd.meshPointCloud;
		}
		if (checkFlags(USE_DEPTH_MESH_POLYGON)) meshPolygon = fd.meshPolygon;
	}

}

void Interface::threadedFunction() {
	RequestPayload pay;
	while (request.receive(pay)) {

		FrameData newFd;
		auto& frames = pipe.wait_for_frames();
		auto& depth = frames.get_depth_frame();
		auto& color = frames.get_color_frame();

		pc.map_to(color);
		filters.filter(depth);
		
		glm::ivec2 depthRes(depth.get_width(), depth.get_height());

		auto& points = pc.calculate(depth);

		if (checkFlags(USE_COLOR_TEXTURE)) {
			newFd.colorPix.setFromPixels(
				(unsigned char*)color.get_data(),
				color.get_width(), color.get_height(), OF_IMAGE_COLOR
			);
		}
		if (checkFlags(USE_DEPTH_TEXTURE)) {
			newFd.depthPix.setFromPixels(
				(float *)points.get_vertices(),
				depthRes.x, depthRes.y, OF_IMAGE_COLOR
			);
		}
		if (checkFlags(USE_DEPTH_MESH_POINTCLOUD)) {
			createPointCloud(newFd.meshPointCloud, points, depthRes, depthPixelSize.get(), false);
		}
		if (checkFlags(USE_DEPTH_MESH_POINTCLOUD_COLOR)) {
			createPointCloud(newFd.meshPointCloud, points, depthRes, depthPixelSize.get(), true);
		}
		if (checkFlags(USE_DEPTH_MESH_POLYGON)) {
			createMesh(newFd.meshPolygon, points, depthRes, depthPixelSize.get());
		}
		
		complete.send(std::move(newFd));
	}

}

void Interface::createPointCloud(ofMesh& mesh, const rs2::points& ps, const glm::ivec2 res, int pixelSize, bool useColor) {

	if (!ps) return;

	mesh.clear();

	const rs2::vertex * vs = ps.get_vertices();
	int pNum = ps.size();

	const int w = res.x;
	const int h = res.y;
	
	if (useColor) {
		auto texCoords = ps.get_texture_coordinates();
		for (int y = 0; y < h - pixelSize; y += pixelSize) {
			for (int x = 0; x < w - pixelSize; x += pixelSize) {
				int i = y * w + x;
				const auto& v = vs[i];
				const auto& uv = texCoords[i];

				if (!v.z) continue;
				mesh.addVertex(glm::vec3(v.x, -v.y, -v.z));
				mesh.addTexCoord(glm::vec2(uv.u, uv.v));
			}
		}
	} else {
		for (int y = 0; y < h - pixelSize; y += pixelSize) {
			for (int x = 0; x < w - pixelSize; x += pixelSize) {
				int i = y * w + x;
				const auto& v = vs[i];
				if (!v.z) continue; continue;
				mesh.addVertex(glm::vec3(v.x, -v.y, -v.z));
			}
		}
	}

}

void Interface::createMesh(ofMesh& mesh, const rs2::points& ps, const glm::ivec2 res, int pixelSize) {

	if (!ps) return;

	mesh.clear();
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);

	const rs2::vertex * vs = ps.get_vertices();

	const int w = res.x;
	const int h = res.y;

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
				if (!v.z) {
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
			}
			else if (eraseCount == 0) {
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
	}

	for (int vi : iList) {
		mesh.addIndex(vi);
	}
}

const ofImage& Interface::getColorImage() const {
	if (!checkFlags(USE_COLOR_TEXTURE)) {
		ofLogError("ofxRealSenseUtil") << "Target flag is disabled!";
	}
	return colorImage;
}
const ofFloatImage& Interface::getDepthImage() const {
	if (!checkFlags(USE_DEPTH_TEXTURE)) {
		ofLogError("ofxRealSenseUtil") << "Target flag is disabled!";
	}
	return depthImage;
}
const ofVboMesh& Interface::getPointCloud() const {
	if (!checkFlags(USE_DEPTH_MESH_POINTCLOUD) && !checkFlags(USE_DEPTH_MESH_POINTCLOUD_COLOR)) {
		ofLogError("ofxRealSenseUtil") << "Target flag is disabled!";
	}
	return meshPointCloud;
}
const ofVboMesh& Interface::getPolygonMesh() const {
	if (!checkFlags(USE_DEPTH_MESH_POLYGON)) {
		ofLogError("ofxRealSenseUtil") << "Target flag is disabled!";
	}
	return meshPolygon;
}