#!/bin/bash

set -x
rm -rf ./platforms/android/app/src/main/java/freego
rm -rf ./platforms/android/app/src/main/java/my
rm -rf ./platforms/android/app/src/main/cpp
rm -rf ./platforms/android/app/CMakeLists.txt
cordova plugin remove novice.cpp.httpserver
cordova plugin add ./FreeGo
# cordova clean android
adb uninstall freenet.cppsvr
cordova run android --device