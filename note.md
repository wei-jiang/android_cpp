vue create ui
cd ui
vue add i18n

"simple-peer": "^9.3.0", //not work
"simple-peer": "^9.1.2",

local http server not https, so no need pwa

./build-android.sh $ANDROID_NDK --arch=arm64-v8a,armeabi-v7a

vscode settings
terminal.integrated.fontFamily:  monospace

//androidX problem:
0 / Remove your android platform
1 / Remove all plugins related to support / gradle / firebase versions control
2 / Add the AndroidX plugins from @dpa99c :
# add plugin to enable AndroidX in the project
cordova plugin add cordova-plugin-androidx
# add plugin to patch existing plugin source that uses the Android Support Library to use AndroidX
cordova plugin add cordova-plugin-androidx-adapter