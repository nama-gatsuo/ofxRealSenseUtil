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

	enum UseFlag {
		USE_TEXTURE_COLOR = (1 << 0),
		USE_TEXTURE_DEPTH = (1 << 1),
		USE_MESH_POINTCLOUD = (1 << 2),
		USE_MESH_POLYGON = (1 << 3)
	};

	class Server : protected ofThread {
	public:
		Server(const std::string& name);
		virtual ~Server();

		void start();
		void stop();

		// Should be called in every frame to
		// fetch data from other thread and prepare data such like ofVboMesh and ofTexture
		virtual void update();

		void enableFlags(uint8_t f) { flags |= f; }
		void disableFlags(uint8_t f) { flags &= ~f; }
		bool checkFlags(uint8_t f) const { return (flags & f) != 0; }

		// Const accessors
		const ofTexture& getColorTex() const;
		const ofTexture& getDepthTex() const;
		const ofVboMesh& getPointCloud() const;
		const ofVboMesh& getPolygonMesh() const;

		// Non-const accessors
		ofVboMesh& getPointCloud() { return const_cast<ofVboMesh&>(const_cast<const Server*>(this)->getPointCloud()); }
		ofVboMesh& getPolygonMesh() { return const_cast<ofVboMesh&>(const_cast<const Server*>(this)->getPolygonMesh()); }

		void setDepthRes(int p) { depthPixelSize.set(p); }
		ofParameterGroup& getParameters() { return rsParams; }

	protected:
		rs2::device device;
		rs2::config config;
		ofPtr<rs2::pipeline> pipe;
		bool bOpen;
		// uint8_t has 8 bits so it can have 8 flags.
		uint8_t flags;

		ofParameterGroup rsParams;
		ofParameterGroup depthMeshParams;
		ofParameter<int> depthPixelSize;
		ofParameter<bool> isClip;
		ofParameter<glm::vec2> p0;
		ofParameter<glm::vec2> p1;

	private:
		void threadedFunction() override;
		
		void createPointCloud(ofMesh& mesh, const rs2::points& ps, const glm::ivec2& res, int pixelStep);
		void createMesh(ofMesh& mesh, const rs2::points& ps, const glm::ivec2& res, int pixelStep);

		struct FrameData {
			ofMesh meshPointCloud;
			ofMesh meshPolygon;
			ofFloatPixels depthPix;
			ofPixels colorPix;
		} fd;

		rs2::frame_queue frameQueue;
		rs2::pointcloud pc;

		ofVboMesh meshPointCloud;
		ofVboMesh meshPolygon;
		ofTexture colorTex;
		ofTexture depthTex;
		bool isNewFrame;

		ofPtr<ofThreadChannel<bool>> request;
		ofPtr<ofThreadChannel<FrameData>> response;

		PostProcessingFilters filters;
	};

}
