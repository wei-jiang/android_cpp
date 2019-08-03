package freego.novice;

import android.content.Intent;
import android.content.Context;
import android.app.Service;
import android.app.PendingIntent;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.os.IBinder;
import android.os.Binder;
import android.os.Build;
import android.util.Log;
import android.os.Bundle;
import android.graphics.Color;
import android.annotation.TargetApi;
import android.os.PowerManager;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import static android.os.PowerManager.PARTIAL_WAKE_LOCK;

import my.free.net.IntVector;
import my.free.net.StringVector;
import my.free.net.FreeNet;

public class ForegroundService extends Service {
    public static final int NOTIFICATION_ID = -574543954;
    private static final String NOTIFICATION_TITLE = "App is running in background";
    private static final String NOTIFICATION_TEXT = "Doing heavy tasks.";
    private static final String NOTIFICATION_ICON = "icon";
    public static final String NOTIFICATION_CHANNEL_ID = "foreground.service.channel"; 
    private static final String LOG_TAG = "cpp_svr";
    private PowerManager.WakeLock wakeLock;
    public static FreeNet mCpp = new FreeNet();
    @Override
    public void onCreate()
    {
        // Log.i(LOG_TAG, "in ForegroundService::onCreate");
        super.onCreate();
        startFGService();
        keepAwake();
        // Log.i(LOG_TAG, "in ForegroundService::onCreate end...............");
    }
    @Override
    public void onDestroy()
    {
        Log.i(LOG_TAG, "in ForegroundService::onDestroy");
        super.onDestroy();
        sleepWell();
        Log.i(LOG_TAG, "in ForegroundService::onDestroy end...............");
    }
    private void keepAwake() {
        PowerManager powerMgr = (PowerManager)getSystemService(POWER_SERVICE);
        wakeLock = powerMgr.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "AudioMix");
        wakeLock.acquire();
    } 
    private void sleepWell()
    {
        stopForeground(true);
        getNotificationManager().cancel(NOTIFICATION_ID);
        if (wakeLock != null) {
            wakeLock.release();
            wakeLock = null;
        }
        stopSelf();
    }
    private NotificationManager getNotificationManager()
    {
        return (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
    }
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(LOG_TAG, "in ForegroundService::onStartCommand");
        String action = intent.getStringExtra("action");
        String s = intent.getStringExtra("args");
        try {
            JSONArray args = new JSONArray(s);
        } catch (JSONException e) {
            Log.d(LOG_TAG, "get json args exception, this should never happen");
        }
        if(action.equals("start")) {
            int ret = mCpp.start_http(CppSvr.listenPort, CppSvr.mAssetsDir);
            Log.i(LOG_TAG, "start cpp http server return="+String.valueOf(ret) );
        } else if( action.equals("start_socks") ){
            mCpp.start_socks(CppSvr.socksPort);
            Log.i(LOG_TAG, "service.start_socks()");
        }
        return START_STICKY;
        // return START_NOT_STICKY;
    }

    private void startFGService() {
        Context context = getApplicationContext();
        String pkgName  = context.getPackageName();
        Intent intent   = context.getPackageManager().getLaunchIntentForPackage(pkgName);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            Log.i(LOG_TAG, "android sdk api >= 26");
            // Delete notification channel if it already exists
            // NotificationManager manager = getNotificationManager();
            // manager.deleteNotificationChannel(NOTIFICATION_CHANNEL_ID);
            // or: NotificationManager.IMPORTANCE_HIGH, NotificationManager.IMPORTANCE_LOW
            Integer importance = NotificationManager.IMPORTANCE_DEFAULT;;
            // Create notification channel
            NotificationChannel channel = new NotificationChannel(NOTIFICATION_CHANNEL_ID, "resource http services", importance);
            channel.setDescription("Enables background processing.");
            channel.enableLights(true);
            channel.setLightColor(Color.parseColor("purple") );
            getSystemService(NotificationManager.class).createNotificationChannel(channel);
        } else {
            Log.i(LOG_TAG, "android sdk api < 26");
        }
        // Get notification icon
        int icon = getResources().getIdentifier(NOTIFICATION_ICON, "drawable", context.getPackageName());
        // Make notification
        Notification.Builder notification = new Notification.Builder(context)
            .setContentTitle(NOTIFICATION_TITLE)
            .setContentText(NOTIFICATION_TEXT)
            .setOngoing(true)
            .setSmallIcon(icon);
        if(Build.VERSION.SDK_INT >= 26){
            notification.setChannelId(NOTIFICATION_CHANNEL_ID);
        }
        if (intent != null ) {
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
            PendingIntent contentIntent = PendingIntent.getActivity(
                    context, NOTIFICATION_ID, intent,
                    PendingIntent.FLAG_UPDATE_CURRENT);
            notification.setContentIntent(contentIntent);
        }
        startForeground(NOTIFICATION_ID, notification.build());
        
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
