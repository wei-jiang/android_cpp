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
import android.Manifest;
import android.content.pm.PackageManager;
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
import java.lang.Runnable;

import android.os.Environment;
import android.net.Uri;
import android.net.ConnectivityManager;
import android.os.PowerManager;
import android.os.Handler;
import android.app.PendingIntent;
import android.content.Intent;
import android.view.View;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;

import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.InterstitialAd;
import com.google.android.gms.ads.MobileAds;

import android.content.Context;
import android.content.ClipboardManager;
import android.content.ClipData;
import android.content.ClipDescription;

public class CppSvr extends CordovaPlugin {
    private static final String AD_UNIT_ID = "ca-app-pub-9524660171794411~5063915451";
    private InterstitialAd interstitialAd;

    protected static final String TAG = "freenet";
    private PowerManager.WakeLock wakeLock = null;
	private PowerManager powerManager = null;
    private Handler handler;
	private PendingIntent wakeupIntent;
	private CordovaWebView webView;
    static {
        // Use the name of the Java wrapper library (which is already linked to the original library)
        System.loadLibrary("cpp_lib");
    }
    String [] permissions = { 
        Manifest.permission.WRITE_EXTERNAL_STORAGE, 
        Manifest.permission.ACCESS_NETWORK_STATE
        // ,Manifest.permission.FOREGROUND_SERVICE
        // ,Manifest.permission.WAKE_LOCK
    };
    static int listenPort;
    CallbackContext cppStartCb, adsCloseCb;
    static String mPubDir;
    static String mMagicPath;
    
    private static final String LOG_TAG = "cpp_svr";
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
    private PluginResult acquire_partial_lock() {
		PluginResult result = null;
		if (this.wakeLock == null) {
			this.wakeLock = this.powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "AudioMix");
			try {
                this.wakeLock.acquire();
                Log.i(LOG_TAG, "acquire PARTIAL_WAKE_LOCK success");
				result = new PluginResult(PluginResult.Status.OK);
			}
			catch( Exception e ) {
                Log.i(LOG_TAG, "acquire PARTIAL_WAKE_LOCK failed", e);
				this.wakeLock = null;
				result = new PluginResult(PluginResult.Status.ERROR,"Can't acquire wake-lock - check your permissions!");
			}
		}
		else {
			result = new PluginResult(PluginResult.Status.ILLEGAL_ACCESS_EXCEPTION,"WakeLock already active - release first");
		}
		return result;
	}
    private final Runnable heartbeat = new Runnable() {
	    public void run() {
	        try {
	        	// Log.d(LOG_TAG, "About to declare ourselves VISIBLE");
	        	mWebView.getEngine().getView().dispatchWindowVisibilityChanged(View.VISIBLE);
	        	// if sdk is 23 (android 6) or greater
				if(android.os.Build.VERSION.SDK_INT > 22){
		            if (wakeLock != null && powerManager != null && powerManager.isDeviceIdleMode()) {
		                Log.i(LOG_TAG, "Poking location service");
		                try {
		                    wakeupIntent.send();
		                } catch (SecurityException e) {
		                    Log.d(LOG_TAG, "SecurityException : Heartbeat location manager keep-alive failed");
		                } catch (PendingIntent.CanceledException e) {
		                    Log.d(LOG_TAG, "PendingIntent.CanceledException : Heartbeat location manager keep-alive failed");
		                }
		            }
		        } else {
                    Log.d(LOG_TAG, "below android 6(api 23)");
                }
	        } finally {
	            if (handler != null) {
	                handler.postDelayed(this, 10000);
	            }
	        }
	    }
    };
    private final Runnable adsTry = new Runnable() {
	    public void run() {
	        interstitialAd.loadAd(new AdRequest.Builder().build());
	    }
	};
    public void initialize(CordovaInterface cordova, CordovaWebView webView) {
        super.initialize(cordova, webView);
        final Context context = cordova.getActivity().getApplicationContext();
        cordova.getActivity().runOnUiThread(
            new Runnable() {
            public void run() {
                // cordova.getActivity().getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                // callbackContext.sendPluginResult(new PluginResult(PluginResult.Status.OK));
                // Initialize the Mobile Ads SDK.
                MobileAds.initialize(context, AD_UNIT_ID);
                // Create the InterstitialAd and set the adUnitId.
                interstitialAd = new InterstitialAd(context);
                // for test
                interstitialAd.setAdUnitId("ca-app-pub-9524660171794411/1096750284");
                interstitialAd.setAdListener(new AdListener() {
                    @Override
                    public void onAdLoaded() {
                        // show("onAdLoaded()");
                    }
                    @Override
                    public void onAdFailedToLoad(int errorCode) {
                        // show("onAdFailedToLoad() with error code: " + errorCode);
                        Log.e(LOG_TAG, "onAdFailedToLoad() with error code: " + errorCode);
                        // interstitialAd.loadAd(new AdRequest.Builder().build());
                        (new Handler()).postDelayed(adsTry, 2*60*1000);
                    }
                    @Override
                    public void onAdClosed() {
                        // Load the next interstitial.
                        interstitialAd.loadAd(new AdRequest.Builder().build());
                        if(adsCloseCb != null) adsCloseCb.success("closed");
                    }
                });
                interstitialAd.loadAd(new AdRequest.Builder().build());
                // ads end 
            }
        });
        
        mWebView = webView;
        assetManager = getActivity().getBaseContext().getAssets();
        this.powerManager = (PowerManager) cordova.getActivity().getSystemService(Context.POWER_SERVICE);
		handler = new Handler();
	    wakeupIntent = PendingIntent.getBroadcast( context , 0, new Intent("com.android.internal.location.ALARM_WAKEUP"), 0);
        self = this;
        try{
            ContextWrapper c = new ContextWrapper(getActivity().getBaseContext());
            mPubDir = c.getFilesDir().getPath() + "/www";
            mMagicPath = c.getFilesDir().getPath() + "/magic.mgc";
            if( new File(mPubDir).exists() ){
                // show(mPubDir + " already exist skip copying");
            } else {
                // copyAssetFolder("www", "/sdcard/www");
                copyAssetFolder("www", mPubDir);
                copyAssetFolder("magic.mgc", mMagicPath);
            }
            // show(mPubDir);
            
        } catch(IOException e){

        }
    }

    private void show(String txt) {
        Toast.makeText(cordova.getActivity().getApplicationContext(), txt, Toast.LENGTH_SHORT).show();
    }
    public void onRequestPermissionResult(int requestCode, String[] permissions, int[] grantResults) throws JSONException
    {
        for(int r:grantResults)
        {
            if(r == PackageManager.PERMISSION_DENIED)
            {
                Log.i(LOG_TAG, "permissions denied, exit-----------------------");
                // this.cppStartCb.sendPluginResult(new PluginResult(PluginResult.Status.ERROR, PERMISSION_DENIED_ERROR));
                getActivity().finish();
                System.exit(0);
            }
        }
        startService();
    }
    void startService(){
        Activity activity = cordova.getActivity();
        Intent intent = new Intent(activity, ForegroundService.class);
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O){
            activity.getApplicationContext().startForegroundService(intent);
            Log.i(LOG_TAG, "activity.getApplicationContext().startForegroundService");
        }else{
            activity.getApplicationContext().startService(intent);
            Log.i(LOG_TAG, "activity.getApplicationContext().startService");
        }
        this.cppStartCb.success("http service started");
        PluginResult result = this.acquire_partial_lock();
        handler.postDelayed(heartbeat, 10000);
        // this.cppStartCb.sendPluginResult(result);
    }
    @Override
    public boolean execute(String action, JSONArray args, CallbackContext callbackContext) throws JSONException {
        Context context = cordova.getActivity().getApplicationContext();
        String packageName = context.getPackageName();
        if (action.equals("copyText")) {
            ClipboardManager clipboard = (ClipboardManager) cordova.getActivity().getSystemService(Context.CLIPBOARD_SERVICE);
            try {
                String text = args.getString(0);
                ClipData clip = ClipData.newPlainText("Text", text);
                clipboard.setPrimaryClip(clip);
                callbackContext.success(text);
                return true;
            } catch (JSONException e) {
                callbackContext.sendPluginResult(new PluginResult(PluginResult.Status.JSON_EXCEPTION));
            } catch (Exception e) {
                callbackContext.sendPluginResult(new PluginResult(PluginResult.Status.ERROR, e.toString()));
            }
        } else if (action.equals("showInterstitialAd")) {
            adsCloseCb = callbackContext;
            cordova.getActivity().runOnUiThread(
                new Runnable() {
                public void run() {
                    if (interstitialAd.isLoaded()) {
                        interstitialAd.show();
                        // Log.i(LOG_TAG, "The interstitial shown ....................");
                    } else {
                        Log.d(LOG_TAG, "The interstitial wasn't loaded yet.");
                        adsCloseCb.success("failed");
                    }
                }
            });
            return true;
        }else if (action.equals("isAndroidVerGt")) {
            int v = args.getInt(0);
            boolean result = Build.VERSION.SDK_INT > v;
            // callbackContext.success(Build.VERSION.SDK_INT>v);
            // Log level: v, d, i, w, e, huawei not show debug info, so change to warning
            // Log.w(LOG_TAG, String.valueOf(Build.VERSION.SDK_INT) + ">" + String.valueOf(v) + " = " + Boolean.toString(result) );
            callbackContext.sendPluginResult(new PluginResult(Status.OK, result));
            return true;
        }else if (action.equals("isAndroidVerLt")) {
            int v = args.getInt(0);
            boolean result = Build.VERSION.SDK_INT < v;
            // callbackContext.success(Build.VERSION.SDK_INT>v);
            callbackContext.sendPluginResult(new PluginResult(Status.OK, result));
            return true;
        }else if (action.equals("isIgnoringBatteryOptimizations")) {
            try {
                if (Build.VERSION.SDK_INT>Build.VERSION_CODES.LOLLIPOP_MR1) {
                    String message ="";
                    PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
                    if (pm.isIgnoringBatteryOptimizations(packageName)) {
                        message ="true";
                    }
                    else
                    {
                        message ="false";
                    }
                    callbackContext.success(message);
                    return true;
                }
                else
                {
                    callbackContext.error("BATTERY_OPTIMIZATIONS Not available.");
                    return false;
                }
            } catch (Exception e) {
                callbackContext.error("IsIgnoringBatteryOptimizations: failed N/A");
                return false;
            }
        }else if (action.equals("requestOptimizations")) {
            try {
                if (Build.VERSION.SDK_INT>Build.VERSION_CODES.LOLLIPOP_MR1) {
                    String message ="Optimizations Requested Successfully";
                    Intent intent = new Intent();
                    intent.setAction(Settings.ACTION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS);
                    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    intent.setData(Uri.parse("package:" + packageName));
                        context.startActivity(intent);

                    callbackContext.success(message);
                    return true;
                }
                else
                {
                    callbackContext.error("BATTERY_OPTIMIZATIONS Not available.");
                    return false;
                }
            } catch (Exception e) {
                callbackContext.error("N/A");
                return false;
            }
        } else if (action.equals("requestOptimizationsMenu")) {
            try {
                if (Build.VERSION.SDK_INT > Build.VERSION_CODES.LOLLIPOP_MR1) {

                    Intent intent = new Intent();
                    PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
                    if (pm.isIgnoringBatteryOptimizations(packageName)){
                        intent.setAction(Settings.ACTION_IGNORE_BATTERY_OPTIMIZATION_SETTINGS);
                        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        context.startActivity(intent);
                    }

                    callbackContext.success("requested");
                    return true;
                }
                else
                {
                    callbackContext.error("BATTERY_OPTIMIZATIONS Not available.");
                    return false;
                }
            } catch (Exception e) {
                callbackContext.error("RequestOptimizationsMenu: failed N/A");
                return false;
            }
        } else if (action.equals("start")) {
            listenPort = args.getInt(0);
            this.cppStartCb = callbackContext;
            if(cordova.hasPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE))
            {
                startService();
            }
            else
            {
                // show("需要访问sd卡以存取文件！");
                cordova.requestPermissions(this, 0, permissions);
            }
        } else if(action.equals("echoAsync")) {
            cordova.getThreadPool().execute(new Runnable() {
                public void run() {
                    String str = "mCpp.get_str()";
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
    private static String inputStreamToString(InputStream in) throws Exception{
        ByteArrayOutputStream result = new ByteArrayOutputStream();
        byte[] buffer = new byte[1024];
        int length;
        while ((length = in.read(buffer)) != -1) {
            result.write(buffer, 0, length);
        }
        // StandardCharsets.UTF_8.name() > JDK 7
        return result.toString("UTF-8");
    }
    private static InputStream filterFileContent(String srcFolder, InputStream in){
        // Log.d(LOG_TAG, srcFolder);
        if( srcFolder.equals("www/index.html") ){
            try {
                String content = inputStreamToString(in);
                // Log.d(LOG_TAG, content);
                content = content.replace("src=cordova.js", "");
                // Log.d(LOG_TAG, content);
                return new ByteArrayInputStream(content.getBytes());
            } catch (Exception e) {
                
            }
        }
        return in;
    }
    private static void copyAssetFile(String srcFolder, String destPath) throws IOException {
        
        InputStream in = assetManager.open(srcFolder);
        // insert filter here
        in = filterFileContent(srcFolder, in);
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
