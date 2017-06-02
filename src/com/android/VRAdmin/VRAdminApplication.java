
package com.android.VRAdmin;

import android.app.ActivityManager;
import android.app.Application;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.SystemClock;
import android.util.Log;
import android.os.PowerManager;

import com.android.VRAdmin.IVRAdminService;

public class VRAdminApplication extends Application {

    private static final String TAG = "VRAdminApplication";
    private static Context context;
    private static PowerManager mPowerMgr = null;
    private static PowerManager.WakeLock mWakeLock = null;
    private static boolean mHoldWakeLock = false;

    public static Context getAppContext() {
        return context;
    }

    public static void onUSBAttached() {
        Log.d(TAG, "onUSBAttached");
    }

    public static void onUSBDetached() {
        Log.d(TAG, "onUSBDetached");
        mPowerMgr.wakeUp(SystemClock.uptimeMillis());
        if (mHoldWakeLock) {
            if (mWakeLock.isHeld())
                mWakeLock.release();
        }
    }

    public static void onPSensorNear() {
        Log.d(TAG, "onPSensorNear");
        mPowerMgr.wakeUp(SystemClock.uptimeMillis());
        if (mHoldWakeLock) {
            if (!mWakeLock.isHeld())
                mWakeLock.acquire();
        }
    }

    public static void onPSensorFar() {
        Log.d(TAG, "onPSensorFar");
        mPowerMgr.goToSleep(SystemClock.uptimeMillis());
        if (mHoldWakeLock) {
            if (mWakeLock.isHeld())
                mWakeLock.release();
        }
    }

    public static void setHoldWakeLock(boolean hold) {
        mHoldWakeLock = hold;
    }

    public static boolean getHoldWakeLock() {
        if (null == mWakeLock)
            return false;
        return mHoldWakeLock;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        context = this;
        mPowerMgr = (PowerManager)getSystemService(Context.POWER_SERVICE);
        mWakeLock = mPowerMgr.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK, "VRAdmin");
    }

    @Override
    public void onTerminate() {
        super.onTerminate();
    }
}
