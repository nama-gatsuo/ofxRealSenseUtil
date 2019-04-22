#pragma once
#include <librealsense2/rs.hpp>
#include "ofVboMesh.h"
#include "ofImage.h"
#include "ofThread.h"
#include "ofThreadChannel.h"
#include <map>

namespace ofxRealSenseUtil {

	enum MeshMode { POLYGON, POINTCLOUD };

	class Interface : public ofThread {
	public:

		Interface();
		~Interface();

		void update();

		const ofImage& getVideo() const {
			return colorImage;
		}
		const ofVboMesh& getMesh() const {
			return mesh;
		}
		void setDepthLimit(float d) {
			payload.depthLimit = d;
		}
		void setDepthRes(int p) {
			payload.pixelSize = p;
		}


	private:
		void threadedFunction() override;
		
		void createPointCloud(ofMesh& mesh, const rs2::points& ps, float depthLimit, int pixelSize);
		void createMesh(ofMesh& mesh, const rs2::points& ps, float depthLimit, int pixelSize);

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
		ofImage colorImage;
		ofImage depthImage;
		bool isNewFrame;

		ofThreadChannel<RequestPayload> request;
		ofThreadChannel<FrameData> complete;
	};
}
