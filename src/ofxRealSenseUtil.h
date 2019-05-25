#pragma once
#include <librealsense2/rs.hpp>
#include "ofVboMesh.h"
#include "ofImage.h"
#include "ofThread.h"
#include "ofThreadChannel.h"
#include "ofParameter.h"
#include "PostProcessingFilter.h"
#include <unordered_map>

namespace ofxRealSenseUtil {

	const static glm::ivec2 rsDepthRes(1280, 720);
	const static glm::ivec2 rsColorRes(1280, 720);

	enum UseFlag {
		USE_COLOR_TEXTURE = (1 << 0),
		USE_DEPTH_TEXTURE = (1 << 1),
		USE_DEPTH_MESH_POINTCLOUD = (1 << 2),
		USE_DEPTH_MESH_POLYGON = (1 << 3)
	}; 

	class Interface : public ofThread {
	public:

		Interface();
		~Interface();

		void update();

		void enableFlags(unsigned char f) { flags |= f; }
		void disableFlags(unsigned char f) { flags &= ~f; }
		bool checkFlags(unsigned char f) const { return (flags & f) != 0; }

		const ofImage& getColorImage() const;
		const ofFloatImage& getDepthImage() const;
		const ofVboMesh& getPointCloud() const;
		const ofVboMesh& getPolygonMesh() const;

		void setDepthRes(int p) { depthPixelSize.set(p); }
		ofParameterGroup& getParameters() { return rsParams; }

	private:
		void threadedFunction() override;
		
		void createPointCloud(ofMesh& mesh, const rs2::points& ps, const glm::ivec2 res, int pixelStep);
		void createMesh(ofMesh& mesh, const rs2::points& ps, const glm::ivec2 res, int pixelStep);

		struct FrameData {
			ofMesh meshPointCloud;
			ofMesh meshPolygon;
			ofFloatPixels depthPix;
			ofPixels colorPix;
		} fd;

		struct RequestPayload {
			unsigned char flags;
		} payload;

		ofParameterGroup rsParams;
		ofParameterGroup depthMeshParams;
		ofParameter<int> depthPixelSize;

		rs2::frame_queue frameQueue;
		rs2::pipeline pipe;
		rs2::pointcloud pc;

		ofVboMesh meshPointCloud;
		ofVboMesh meshPolygon;
		ofImage colorImage;
		ofFloatImage depthImage;
		bool isNewFrame;

		// unsignd char has 8 bits so it can have 8 falgs.
		unsigned char flags; 

		ofThreadChannel<RequestPayload> request;
		ofThreadChannel<FrameData> complete;

		PostProcessingFilters filters;
	};
}
