#include "RealSenseServer.h"

using namespace ofxRealSenseUtil;

Server::Server() {
	rsParams.add(filters.getParameters());
	depthMeshParams.setName("depthMeshParams");
	depthMeshParams.add(depthPixelSize.set("pixelSize", 10, 1, 100));
	rsParams.add(depthMeshParams);
	rsParams.add(isClip.set("enableClip", false));
}

Server::~Server() {}

void Server::start() {
	request = std::make_shared<ofThreadChannel<bool>>();
	response = std::make_shared<ofThreadChannel<FrameData>>();
	pipe->start(config);
	device = pipe->get_active_profile().get_device();
	ofLogNotice("ofxRealSenseUtil") << "Start: " << device.get_info(RS2_CAMERA_INFO_NAME);
	startThread();
}

void Server::stop() {
	request->close();
	response->close();
	waitForThread(true);
	stopThread();
	pipe->stop();
}

void Server::update() {
	bool r = true;
	request->send(r);
	isNewFrame = false;

	while (response->tryReceive(fd)) {
		isNewFrame = true;
	}

	if (isNewFrame) {
		if (checkFlags(USE_COLOR_TEXTURE)) colorImage.setFromPixels(fd.colorPix);
		if (checkFlags(USE_DEPTH_TEXTURE)) depthImage.setFromPixels(fd.depthPix);
		if (checkFlags(USE_DEPTH_MESH_POINTCLOUD)) meshPointCloud = fd.meshPointCloud;
		if (checkFlags(USE_DEPTH_MESH_POLYGON)) meshPolygon = fd.meshPolygon;
	}

}

void Server::refreshConfig(const Settings& s) {
	config = rs2::config();
	pipe = std::make_shared<rs2::pipeline>();

	if (s.deviceId != -1) {
		rs2::context ctx;
		rs2::device_list deviceList = ctx.query_devices();

		if (deviceList.size() == 0) {
			//throw std::runtime_error("No device detected. Is it plugged in?");
			ofLogWarning("ofxRealSenseUtil") << "No device detected. Is realsense plugged in?";
		} else if (deviceList.size() <= s.deviceId) {
			ofLogWarning("ofxRealSenseUtil") << "Device id (" << s.deviceId << ") is not available.";
		} else {
			device = deviceList[s.deviceId];
			const std::string& deviceSerial = device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
			config.enable_device(deviceSerial);
			ofLogNotice("ofxRealSenseUtil") << deviceSerial << " is active!";
		}
	} else {
		ofLogNotice("ofxRealSenseUtil") << "Device is not set.";
	}

	if (s.useDepth) config.enable_stream(RS2_STREAM_DEPTH, s.depthRes.x, s.depthRes.y, RS2_FORMAT_Z16, 30);
	if (s.useColor) config.enable_stream(RS2_STREAM_COLOR, s.colorRes.x, s.colorRes.y, RS2_FORMAT_RGB8, 30);

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

		if (checkFlags(USE_COLOR_TEXTURE)) {
			newFd.colorPix.setFromPixels(
				(unsigned char*)color.get_data(),
				color.get_width(), color.get_height(), OF_IMAGE_COLOR
			);
		}

		glm::ivec2 depthRes(depth.get_width(), depth.get_height());
		auto& points = pc.calculate(depth);
		if (checkFlags(USE_DEPTH_TEXTURE)) {
			newFd.depthPix.setFromPixels(
				(float*)points.get_vertices(),
				depthRes.x, depthRes.y, OF_IMAGE_COLOR
			);
		}
		
		if (checkFlags(USE_DEPTH_MESH_POINTCLOUD)) {
			createPointCloud(newFd.meshPointCloud, points, depthRes, depthPixelSize.get());
		}
		if (checkFlags(USE_DEPTH_MESH_POLYGON)) {
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
	int pNum = ps.size();
	
	const int w = res.x;
	const int h = res.y;
		
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

}

void Server::createMesh(ofMesh& mesh, const rs2::points& ps, const glm::ivec2& res, int pixelSize) {

	if (!ps) return;
	
	mesh.clear();
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);

	const rs2::vertex * vs = ps.get_vertices();
	const rs2::texture_coordinate * texCoords = ps.get_texture_coordinates();

	glm::ivec2 start(0, 0), end(res);
	if (isClip) {
		glm::vec2 scale = glm::vec2(res) / glm::vec2(rsDepthRes);
		start = clipRect.position * scale;
		end = clipRect.getBottomRight() * scale;
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

const ofImage& Server::getColorImage() const {
	if (!checkFlags(USE_COLOR_TEXTURE)) {
		ofLogError("ofxRealSenseUtil") << "Target flag is disabled!";
	}
	return colorImage;
}
const ofFloatImage& Server::getDepthImage() const {
	if (!checkFlags(USE_DEPTH_TEXTURE)) {
		ofLogError("ofxRealSenseUtil") << "Target flag is disabled!";
	}
	return depthImage;
}
const ofVboMesh& Server::getPointCloud() const {
	if (!checkFlags(USE_DEPTH_MESH_POINTCLOUD)) {
		ofLogError("ofxRealSenseUtil") << "Target flag is disabled!";
	}
	return meshPointCloud;
}
const ofVboMesh& Server::getPolygonMesh() const {
	if (!checkFlags(USE_DEPTH_MESH_POLYGON)) {
		ofLogError("ofxRealSenseUtil") << "Target flag is disabled!";
	}
	return meshPolygon;
}