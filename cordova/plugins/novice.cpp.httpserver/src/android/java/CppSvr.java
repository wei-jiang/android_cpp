package freego.novice;

import android.app.Activity;
import android.widget.Toast;
import android.annotation.SuppressLint;
import android.util.Base64;
// import android.content.BroadcastReceiver;
// import android.content.Context;
// import android.content.Intent;
// import android.content.IntentFilter;
import android.content.*;
import android.content.res.AssetManager;

import android.graphics.Bitmap;
import android.os.Build;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;
import android.os.Handler;
import android.os.Message;

import org.apache.cordova.CordovaArgs;
import org.apache.cordova.CordovaWebView;
import org.apache.cordova.CordovaInterface;
import org.apache.cordova.PluginResult;
import org.apache.cordova.PluginResult.Status;

import org.apache.cordova.CordovaPlugin;
import org.apache.cordova.CallbackContext;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import static java.util.Arrays.copyOfRange;
import java.io.*;
import java.util.*;

import my.free.net.IntVector;
import my.free.net.StringVector;
import my.free.net.FreeNet;


public class CppSvr extends CordovaPlugin {
    protected static final String TAG = "freenet";
    static {
        // Use the name of the Java wrapper library (which is already linked to the original library)
        System.loadLibrary("cpp_lib");
    }
    private String mPubDir;
    private String mMagicPath;
    private FreeNet mCpp;
    private CordovaWebView mWebView;
    private static AssetManager assetManager = null;
    private static CppSvr self;
    public static String byteArrayToStr(byte[] byteArray) {
        if (byteArray == null) {
            return null;
        }
        String str = new String(byteArray);
        return str;
    }

    public void initialize(CordovaInterface cordova, CordovaWebView webView) {
        super.initialize(cordova, webView);
        mWebView = webView;
        assetManager = getActivity().getBaseContext().getAssets();
        self = this;
        mCpp = new FreeNet();
        try{
            ContextWrapper c = new ContextWrapper(getActivity().getBaseContext());
            mPubDir = c.getFilesDir().getPath() + "/www";
            mMagicPath = c.getFilesDir().getPath() + "/magic.mgc";
            if( new File(mPubDir).exists() ){
                show(mPubDir + " already exist skip copying");
            } else {
                // copyAssetFolder("www", "/sdcard/www");
                copyAssetFolder("www", mPubDir);
                copyAssetFolder("magic.mgc", mMagicPath);
            }
            show(mPubDir);
            
        } catch(IOException e){

        }
    }

    private void show(String txt) {
        Toast.makeText(cordova.getActivity().getApplicationContext(), txt, Toast.LENGTH_SHORT).show();
    }

    @Override
    public boolean execute(String action, JSONArray args, CallbackContext callbackContext) throws JSONException {
        if (action.equals("start")) {
            int port = args.getInt(0);
            int ret = mCpp.start_svr(port, mPubDir);
            // show("start c++ server return : " +  ret);
            callbackContext.success("start c++ server return : " +  ret);
            // callbackContext.sendPluginResult(new PluginResult(PluginResult.Status.OK, ret ));

        } else if(action.equals("echoAsync")) {
            cordova.getThreadPool().execute(new Runnable() {
                public void run() {
                    String str = mCpp.get_str();
                    callbackContext.sendPluginResult( 
                        new PluginResult(
                            PluginResult.Status.OK, args.optString(0)+" from java; cpp ret="+str
                        )
                    );
                }
            });
        } else {
            return false;
        }
        return true;
    }

    private static void copyAssetFolder(String srcFolder, String destPath) throws IOException {
        String[] files = assetManager.list(srcFolder);
        if (files.length == 0) {
            // Copy the file
            copyAssetFile(srcFolder, destPath);
        } else {
            // Create the folder
            new File(destPath).mkdirs();
            for (String file : files) {
                //filter out cordova things
                if( file.toLowerCase().contains("cordova") || file.toLowerCase().contains("plugins") ) {
                    // self.show(srcFolder);
                    continue;
                } 
                copyAssetFolder(srcFolder + "/" + file, destPath + "/" + file);
            }
        }
    }

    private static void copyAssetFile(String srcFolder, String destPath) throws IOException {
          
        InputStream in = assetManager.open(srcFolder);
        new File(destPath).createNewFile();
        OutputStream out = new FileOutputStream(destPath);
        copyFile(in, out);
        in.close();
        in = null;
        out.flush();
        out.close();
        out = null;
    }

    private static void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }
    }

    private Activity getActivity() {
        return this.cordova.getActivity();
    }

    private void shortToast(String message) {
        if (message != null && message.length() > 0) {
            Toast.makeText(cordova.getActivity().getApplicationContext(), message, Toast.LENGTH_SHORT).show();
        }
    }

    private void longToast(String message) {
        if (message != null && message.length() > 0) {
            Toast.makeText(cordova.getActivity().getApplicationContext(), message, Toast.LENGTH_LONG).show();
        }
    }

}
