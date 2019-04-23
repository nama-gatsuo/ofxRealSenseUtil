#include "ofxRealSenseUtil.h"

using namespace ofxRealSenseUtil;

Interface::Interface() : isNewFrame(true), flags(0) {
	payload.flags = flags;

	rs2::context ctx;
	auto& list = ctx.query_devices(); // Get a snapshot of currently connected devices
	if (list.size() == 0) {
		//throw std::runtime_error("No device detected. Is it plugged in?");
	} else {
		startThread();
		pipe.start();
	}

	rsParams.setName("ofxRealSenseUtil");
	depthMeshParams.setName("depthMeshParams");
	depthMeshParams.add(depthPixelSize.set("pixelSize", 3, 1, 10));
	depthMeshParams.add(depthZLimit.set("depthZLimit(by meter)", 3., 0., 10.));
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
		if (checkFlags(USE_DEPTH_MESH_POINTCLOUD)) meshPointCloud = fd.meshPointCloud;
		if (checkFlags(USE_DEPTH_MESH_POLYGON)) meshPolygon = fd.meshPolygon;
	}

}

void Interface::threadedFunction() {
	RequestPayload pay;
	while (request.receive(pay)) {

		FrameData newFd;
		auto& frames = pipe.wait_for_frames();
		auto& depth = frames.get_depth_frame();
		auto& video = frames.get_color_frame();
		auto& points = pc.calculate(depth);

		if (checkFlags(USE_COLOR_TEXTURE)) {
			newFd.colorPix.setFromPixels(
				(unsigned char *)video.get_data(),
				video.get_width(), video.get_height(), OF_IMAGE_COLOR
			);
		}
		if (checkFlags(USE_DEPTH_TEXTURE)) {
			newFd.depthPix.setFromPixels(
				(unsigned char *)depth.get_data(),
				depth.get_width(), depth.get_height(), OF_IMAGE_COLOR
			);
		}
		if (checkFlags(USE_DEPTH_MESH_POINTCLOUD)) {
			createPointCloud(newFd.meshPointCloud, points, depthZLimit.get(), depthPixelSize.get());
		}
		if (checkFlags(USE_DEPTH_MESH_POLYGON)) {
			createMesh(newFd.meshPolygon, points, depthZLimit.get(), depthPixelSize.get());
		}

		complete.send(std::move(newFd));
	}

}

void Interface::createPointCloud(ofMesh& mesh, const rs2::points& ps, float depthLimit, int pixelSize) {

	mesh.clear();

	const rs2::vertex * vs = ps.get_vertices();
	int pNum = ps.size();

	const int w = rsDepthRes.x;
	const int h = rsDepthRes.y;

	for (int y = 0; y < h - pixelSize; y += pixelSize) {
		for (int x = 0; x < w - pixelSize; x += pixelSize) {
			int i = y * w + x;
			const auto& v = vs[i];
			if (isinf(v.z) || v.z > depthLimit || v.z < 0.1) continue;
			mesh.addVertex(glm::vec3(v.x, -v.y, -v.z));
		}
	}

}

void Interface::createMesh(ofMesh& mesh, const rs2::points& ps, float depthLimit, int pixelSize) {

	mesh.clear();
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);

	const rs2::vertex * vs = ps.get_vertices();

	const int w = rsDepthRes.x;
	const int h = rsDepthRes.y;

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
}