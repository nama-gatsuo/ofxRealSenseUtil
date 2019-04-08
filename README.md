# ofxRealSenseUtil
Wrapper and Utilities for RealSense SDK

## Dependency
* IntelRealSense / [librealsense](https://github.com/IntelRealSense/librealsense)

## Feature
* Polygon (triangulated geometry) creation in separated thread
* Linkage to the path of SDK location in Build Solution (Windows only)

## TODO
* Mac compatibility (But [this addon](https://github.com/hiroMTB/ofxRealsense2) seems very nice for mac)
* Auto copy dlls into `/bin`. Because of lack of this function, currently we have to do either one of below items manually.
  1. copy and paste `realsense2.dll` and `Intel.Realsense.dll` files from RealSense SDK directory's `bin` into your `OF_PJ/bin`.
  2. In your project properties, go to Debugging > Environment and add `$(PROGRAMFILES)/Intel RealSense SDK 2.0/lib/$(Platform)/`. This is kind of a search path for dll in run time. `OF_PJ/bin` is a default search path for searching dlls, so your app should work if dlls are located there.
