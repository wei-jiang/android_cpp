vue create ui
cd ui
vue add i18n

"simple-peer": "^9.3.0", //not work
"simple-peer": "^9.1.2",
npm install simple-peer@9.1.2
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

openssl req -x509 -nodes -days 36500 -newkey rsa:4096 -keyout py.key -out py.csr \
-subj "/C=CN/ST=HuNan/L=ChangSha/O=PiaoYun Inc. /OU=IT Department/CN=piaoyun.shop"

openssl req -nodes -newkey rsa:2048 -keyout py.key -out py.csr -subj "/C=CN/ST=HuNan/L=ChangSha/O=piaoyun/OU=IT Department/CN=piaoyun.shop"

openssl x509 -req -days 36500 -in py.csr -signkey py.key -out py.crt

openssl x509 \
       -signkey py.key \
       -in py.csr \
       -req -days 2048 -out py.crt

openssl req -nodes -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365

openssl req -x509 \
 -nodes -days 36500 -newkey rsa:4096 \
 -keyout py.key \
 -out py.crt \
 -subj "/C=CN/ST=HuNan/L=ChangSha/O=piaoyun/OU=IT Department/CN=piaoyun.shop"

git checkout master~1            # Checkout previous commit on master
git checkout -b new_master       # Create branch for new master
git branch -D master             # Delete old master
git branch -mv new_master master # Make new_master master
<!-- or -->
git reset --hard master~1

echo -n "hello" | nc -4u 222.244.70.87 22080

echo -n "hello" | nc -4u 139.155.50.166 34024