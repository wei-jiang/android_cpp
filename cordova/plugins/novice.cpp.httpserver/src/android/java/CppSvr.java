package freego.novice;

import android.app.Activity;
import android.os.IBinder;
import android.widget.Toast;
import android.annotation.SuppressLint;
import android.util.Base64;
// import android.content.BroadcastReceiver;
// import android.content.Context;
// import android.content.IntentFilter;
// import android.content.ComponentName;
// import android.content.Intent;
// import android.content.ServiceConnection;
import android.content.*;
import android.content.res.AssetManager;
import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Build;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;
import android.util.DisplayMetrics;
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
import android.widget.FrameLayout;
// import android.view.ViewGroup.LayoutParams;
import android.widget.FrameLayout.LayoutParams;
import android.view.Gravity;
import android.widget.TextView;

import android.graphics.Color;

import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.InterstitialAd;
import com.google.android.gms.ads.MobileAds;

import android.content.Context;
import android.content.ClipboardManager;
import android.content.ClipData;
import android.content.ClipDescription;


public class CppSvr extends CordovaPlugin {
    public static final int REQUEST_CODE = 0x0ba7c0de;
    private static final String ENCODE = "encode";
    private static final String CANCELLED = "cancelled";
    private static final String FORMAT = "format";
    private static final String TEXT = "text";
    public static final String CAMERA = Manifest.permission.CAMERA;
    public static final int CAMERA_REQ_CODE = 0x510719;
    public static final int PERMISSION_DENIED_ERROR = 20;
    // above for qr scan
    private static final String AD_UNIT_ID = "ca-app-pub-9524660171794411~5063915451";
    private InterstitialAd interstitialAd;

    protected static final String TAG = "freenet";
    private PowerManager.WakeLock wakeLock = null;
	private PowerManager powerManager = null;
    private Handler handler;
	private PendingIntent wakeupIntent;
    private CordovaWebView webView;
    private AdView mAdView;
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
    static int listenPort, socksPort;
    CallbackContext cppStartCb, adsCloseCb;
    static String mAssetsDir;
    
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
    @Override
    public void onDestroy()
    {
        super.onDestroy();
        Log.i(LOG_TAG, "CppSvr::onDestroy()");
        android.os.Process.killProcess(android.os.Process.myPid());
      
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
		                // Log.i(LOG_TAG, "Poking location service");
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
    boolean isBannerLoaded = false;

    private void showBanner(){
        if(mAdView.getParent() != null) return;
        FrameLayout layout = (FrameLayout) mWebView.getView().getParent();
        LayoutParams params = new LayoutParams(LayoutParams.MATCH_PARENT, 200, Gravity.TOP);
        LayoutParams wvLayoutParas = (LayoutParams) mWebView.getView().getLayoutParams();
        wvLayoutParas.gravity = Gravity.BOTTOM;
        // ----------------------
        DisplayMetrics displayMetrics = new DisplayMetrics();
        cordova.getActivity().getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        int height = displayMetrics.heightPixels;
        int width = displayMetrics.widthPixels;
        // dp
        // DisplayMetrics displayMetrics = cordova.getActivity().getResources().getDisplayMetrics();    
        // float dpHeight = displayMetrics.heightPixels / displayMetrics.density;
        // float dpWidth = displayMetrics.widthPixels / displayMetrics.density;
        // ----------------------------
        // wvLayoutParas.height = height - 300;
        // params.setMargins(left, top, right, bottom);
        wvLayoutParas.height = LayoutParams.MATCH_PARENT;
        wvLayoutParas.setMargins(0, 210, 0, 0);
        mAdView.setLayoutParams(params);   
        // ((ViewGroup)mAdView.getParent()).removeView(mAdView);
        layout.addView(mAdView);
        layout.setBackgroundColor(Color.rgb(175, 245, 245));
        layout.requestLayout();
    }
    private void loadBanner(){
        Context context = cordova.getActivity();
        mAdView = new AdView(context);
        mAdView.setAdListener(new AdListener() {
            @Override
            public void onAdLoaded() {
                // Code to be executed when an ad finishes loading.
                Log.i(LOG_TAG, "onAdLoaded()-----------------------");
                isBannerLoaded = true;
            }
            @Override
            public void onAdFailedToLoad(int errorCode) {
                // Code to be executed when an ad request fails.
                Log.i(LOG_TAG, "onAdFailedToLoad(), errorCode="+String.valueOf(errorCode));
                if(mAdView.getParent() == null){
                    (new Handler()).postDelayed(new Runnable() {
                        public void run() {
                            mAdView.loadAd(new AdRequest.Builder().build());
                        }
                    }, 1*60*1000);
                }
            }
            @Override
            public void onAdOpened() {
                // Code to be executed when an ad opens an overlay that
                // covers the screen.
                Log.i(LOG_TAG, "onAdOpened()-----------------------");
            }
            @Override
            public void onAdClicked() {
                // Code to be executed when the user clicks on an ad.
                Log.i(LOG_TAG, "onAdClicked()-----------------------");
            }       
            @Override
            public void onAdLeftApplication() {
                // Code to be executed when the user has left the app.
                Log.i(LOG_TAG, "onAdLeftApplication()-----------------------");
            }
            @Override
            public void onAdClosed() {
                // Code to be executed when the user is about to return
                // to the app after tapping on an ad.
                Log.i(LOG_TAG, "onAdClosed()-----------------------");
            }
        });
        mAdView.setAdSize(AdSize.SMART_BANNER);
        mAdView.setAdUnitId("ca-app-pub-9524660171794411/7360869758");     
        AdRequest adRequest = new AdRequest.Builder().build();
        mAdView.loadAd(adRequest);       
    }
    public void initialize(CordovaInterface cordova, CordovaWebView webView) {
        super.initialize(cordova, webView);
        mWebView = webView;
        final Context context = cordova.getActivity().getApplicationContext();
        cordova.getActivity().runOnUiThread(
            new Runnable() {
            public void run() {
                // cordova.getActivity().getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                // callbackContext.sendPluginResult(new PluginResult(PluginResult.Status.OK));
                // Initialize the Mobile Ads SDK.
                MobileAds.initialize(context, AD_UNIT_ID);
                // banner ads
                loadBanner();
                // Create the InterstitialAd and set the adUnitId.
                interstitialAd = new InterstitialAd(context);
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
        assetManager = getActivity().getBaseContext().getAssets();
        this.powerManager = (PowerManager) cordova.getActivity().getSystemService(Context.POWER_SERVICE);
		handler = new Handler();
	    wakeupIntent = PendingIntent.getBroadcast( context , 0, new Intent("com.android.internal.location.ALARM_WAKEUP"), 0);
        self = this;
        try{
            ContextWrapper c = new ContextWrapper(getActivity().getBaseContext());
            mAssetsDir = c.getFilesDir().getPath();
            String wwwPath = mAssetsDir + "/www";
            String magicPath = mAssetsDir + "/magic.mgc";
            if( !(new File(wwwPath).exists()) ) copyAssetFolder("www", wwwPath);
            if( !(new File(magicPath).exists()) ) copyAssetFolder("magic.mgc", magicPath);
            // show(mAssetsDir);
            
        } catch(IOException e){

        }
    }

    private void show(String txt) {
        Toast.makeText(cordova.getActivity().getApplicationContext(), txt, Toast.LENGTH_SHORT).show();
    }
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == REQUEST_CODE) {
            if (resultCode == Activity.RESULT_OK) {
                JSONObject obj = new JSONObject();
                try {
                    obj.put(TEXT, data.getStringExtra("qr_code"));
                    obj.put(FORMAT, data.getStringExtra("format"));
                    obj.put(CANCELLED, false);
                } catch (JSONException e) {
                    Log.d(LOG_TAG, "This should never happen");
                }
                // this.success(new PluginResult(PluginResult.Status.OK, obj), this.callback);
                this.scan_cb.success(obj);
            } else if (resultCode == Activity.RESULT_CANCELED) {
                JSONObject obj = new JSONObject();
                try {
                    obj.put(TEXT, "");
                    obj.put(FORMAT, "");
                    obj.put(CANCELLED, true);
                } catch (JSONException e) {
                    Log.d(LOG_TAG, "This should never happen");
                }
                // this.success(new PluginResult(PluginResult.Status.OK, obj), this.callback);
                this.scan_cb.success(obj);
            } else {
                // this.error(new PluginResult(PluginResult.Status.ERROR), this.callback);
                this.scan_cb.error("Unexpected error");
            }
        }
    }
    public void onRequestPermissionResult(int requestCode, String[] permissions, int[] grantResults) throws JSONException
    {
        if(requestCode == CAMERA_REQ_CODE){
            if(grantResults[0] == PackageManager.PERMISSION_GRANTED){
                start_scan();
            }
            return;
        }
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
    CallbackContext scan_cb;
    @Override
    public boolean execute(String action, JSONArray args, CallbackContext callbackContext) throws JSONException {
        Context context = cordova.getActivity().getApplicationContext();
        String packageName = context.getPackageName();
        if (action.equals("start_socks")) {
            socksPort = args.getInt(0);
            ForegroundService.mCpp.start_socks_proxy(socksPort);
            Log.i(LOG_TAG, "service.start_socks()");
            callbackContext.sendPluginResult(new PluginResult(Status.OK, 0));
            // return true;            
        } else if (action.equals("showBanner")) {
            cordova.getActivity().runOnUiThread(
                new Runnable() {
                public void run() {
                    if(isBannerLoaded){
                        showBanner();
                        // Log.i(LOG_TAG, "isBannerLoaded="+Boolean.toString(isBannerLoaded) );
                        callbackContext.success("ok");
                    } else{
                        // Log.i(LOG_TAG, "isBannerLoaded="+Boolean.toString(isBannerLoaded) );
                        callbackContext.error("fail");
                    }
                }
            });
            // return true;            
        } else if (action.equals("copyText")) {
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
        } else if (action.equals("restart")) {
            listenPort = args.getInt(0);
            int ret = ForegroundService.mCpp.start_svr(CppSvr.listenPort, CppSvr.mAssetsDir);
            Log.i(LOG_TAG, "service.startSvr()=" + ret);
            callbackContext.sendPluginResult(new PluginResult(Status.OK, ret));      
        } else if(action.equals("echoAsync")) {
            cordova.getThreadPool().execute(new Runnable() {
                public void run() {
                    String str = ForegroundService.mCpp.get_str();
                    callbackContext.sendPluginResult( 
                        new PluginResult(
                            PluginResult.Status.OK, args.optString(0)+" from java; cpp ret="+str
                        )
                    );
                }
            });
        } else if (action.equals("scan_by_camera")) {
            // Log.i(LOG_TAG, "in scan_by_camera");
            scan_cb = callbackContext;
            if (cordova.hasPermission(CAMERA)) {
                start_scan();
            } else {
                cordova.requestPermission(this, CAMERA_REQ_CODE, CAMERA);
            }
            // callbackContext.success(); // Thread-safe.
            return true;
        }else {
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
    private void start_scan() {
        cordova.setActivityResultCallback(this);
        cordova.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Context context = cordova.getActivity().getApplicationContext();
                Intent intent = new Intent(context, Scanner.class);
                cordova.getActivity().startActivityForResult(intent, REQUEST_CODE);
                // cordova.setActivityResultCallback (self);
                // cordova.startActivityForResult(self, intent, 0);
            }
        });
    }
}
