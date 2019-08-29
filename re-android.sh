#/bin/bash

cd cordova
cordova platform rm android
cordova platform add android@8
sed -i '/\/\/ debug = false/!b;n;c handler.proceed();' \
/data/github/net/cordova/platforms/android/CordovaLib/src/org/apache/cordova/engine/SystemWebViewClient.java