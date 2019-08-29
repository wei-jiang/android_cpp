#!/bin/bash
export CORDOVA_ANDROID_GRADLE_DISTRIBUTION_URL=file:///home/novice/tools/gradle-4.10.3-all.zip
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
export PATH=$JAVA_HOME/bin:$PATH
export ANDROID_HOME="/data/android/sdk"
export ANDROID_SDK_ROOT="/data/android/sdk"
set -x
rm -rf ./platforms/android/app/src/main/java/freego
rm -rf ./platforms/android/app/src/main/java/my
rm -rf ./platforms/android/app/src/main/cpp
rm -rf ./platforms/android/app/CMakeLists.txt
cordova plugin remove novice.cpp.httpserver
cordova plugin add ./py-plugin
cordova clean android

# adb uninstall freenet.cppsvr
cordova run android --release --device
# cordova build android


# sed -i '/\/\/ debug = false/!b;n;c handler.proceed();' \
# /data/github/net/cordova/platforms/android/CordovaLib/src/org/apache/cordova/engine/SystemWebViewClient.java