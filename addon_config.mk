meta:
	ADDON_NAME = ofxRealSense
	ADDON_DESCRIPTION = Addon for RealSense
	ADDON_AUTHOR = Ayumu Nagamatsu
	ADDON_TAGS = realsense
	ADDON_URL = https://github.com/nama-gatsuo/ofxRealSenseUtil

common:

vs:
	ADDON_INCLUDES += "$(PROGRAMFILES)/Intel RealSense SDK 2.0/include/"
	ADDON_LIBS += "$(PROGRAMFILES)/Intel RealSense SDK 2.0/lib/$(Platform)/realsense2.lib"
	ADDON_DLLS_TO_COPY += "$(PROGRAMFILES)/Intel RealSense SDK 2.0/bin/x64/Intel.Realsense.dll"
	ADDON_DLLS_TO_COPY += "$(PROGRAMFILES)/Intel RealSense SDK 2.0/bin/x64/realsense2.dll"
