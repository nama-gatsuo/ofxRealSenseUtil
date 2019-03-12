# ofxRealSenseUtil
Wrapper and Utilities for RealSense SDK

## Dependency
* IntelRealSense / [librealsense](https://github.com/IntelRealSense/librealsense)

## Feature
* Polygon (triangulated geometry) creation in separated thread
* Linkage to the path of SDK location in Build Solution (Windows only)

## TODO
* Mac compatibility
* Auto copy dlls into `/bin`
  Currently we have to copy and paste dll files into your `/bin` from RealSense original SDK manually.
  `realsense2.dll` and `Intel.Realsense.dll` will be needed to compile.
