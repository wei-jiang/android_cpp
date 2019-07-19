#!/bin/bash
export CORDOVA_ANDROID_GRADLE_DISTRIBUTION_URL=file:///home/novice/tools/gradle-4.10.3-all.zip
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
export PATH=$JAVA_HOME/bin:$PATH
export ANDROID_HOME="/data/android/sdk"
export ANDROID_SDK_ROOT="/data/android/sdk"
CurDir="$(dirname "$PWD")"
set -x
rm -rf ./platforms/android/app/src/main/java/freego
rm -rf ./platforms/android/app/src/main/java/my
rm -rf ./platforms/android/app/src/main/cpp
rm -rf ./platforms/android/app/CMakeLists.txt
cordova plugin remove novice.cpp.httpserver
cordova plugin add ./py-plugin
cordova clean android

cordova build android --release --prod -- --versionCode=$(date +"%Y%m%d")
# vanilla gradle use: -PversionCode=2019071902
cd platforms/android && ./gradlew bundleRelease -PcdvVersionCode="$(date +"%Y%m%d")02" -PaabBundle=1
cp -f app/build/outputs/apk/release/*.apk $CurDir/dist/
cp -f app/build/outputs/bundle/release/*.aab $CurDir/dist/
cp -f $CurDir/dist/*.{apk,aab} /home/novice/piaoyun/dist/
