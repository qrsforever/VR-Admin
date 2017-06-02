
package com.android.VRAdmin;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.SystemClock;

import android.util.Log;

import java.lang.ref.WeakReference;

public class VRManager {
    private static final String TAG = "VRManager";

    private static VRManager mVRMgr = null;
    private Context mContext = null;

    static {
        System.loadLibrary("vrnative_jni");
        native_init();
    }

    private static native final void native_init();
    private native final void native_setup(Object refobj);

    private native String native_getSystemNode(String path);
    private native int native_setSystemNode(String path, String value);
    private native String native_systemCall(String cmd);
    private native String native_getProperty(String key);
    private native int native_setProperty(String key, String value);
    private native int native_setThreadScheduler(int tid, int policy, int prio);

    private VRManager(Context context) {
        Log.d(TAG, "VRManager");
        native_setup(new WeakReference<VRManager>(this));
        mContext = context;
    }

    public static VRManager getInstance(Context context) {
        if (mVRMgr == null) {
            mVRMgr = new VRManager(context);
        }
        return mVRMgr;
    }

    public String getSystemNode(String path) {
        return native_getSystemNode(path);
    }

    public int setSystemNode(String path, String value) {
        return native_setSystemNode(path, value);
    }

    public String systemCall(String cmd) {
        return native_systemCall(cmd);
    }

    public String getProperty(String key) {
        return native_getProperty(key);
    }

    public int setProperty(String key, String value) {
        return native_setProperty(key, value);
    }

    public int setThreadScheduler(int tid, int policy, int pri) {
        return native_setThreadScheduler(tid, policy, pri);
    }

    private static void postEventFromNative(Object vr, int what, int arg1, int arg2, String str) {
        Log.d(TAG, "postEventFromNative(" + what + "," + arg1 + "," + arg2 + "," + str + ")");
        VRManager vrmgr = (VRManager)((WeakReference)vr).get();
        if (vrmgr == null)
            return;
    }
}
