/* //device/content/providers/media/src/com/android/providers/media/MediaScannerService.java
 **
 ** Copyright 2007, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

package com.android.VRAdmin;

import android.app.KeyguardManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
//import java.lang.ProcessBuilder;
//import java.lang.Process;

import com.android.VRAdmin.IVRAdminService;
import android.os.IBinder;
import android.os.PowerManager;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.SystemClock;
import android.util.Log;
import java.util.ArrayList;

import android.net.LocalServerSocket;
import android.net.LocalSocket;

public class VRAdminService extends Service {
    private static final String TAG = "VRAdminService";

    private static final String MSM_CMD_AUTOREFRESH = "/sys/class/graphics/fb0/msm_cmd_autorefresh_en";
    private int mClientCount = 0;

    private static final String VR_GRALLOC_PROPERTY = "vr.gralloc.gfx_ubwc_disable";
    private boolean bRunning = false;

    private VRManager mVRService = null;
    private PSensorUtility mPSensorUtility = null;
    //@@@ yujie add for [COOLMAX-912] begin
    private final long SLEEP_DELAY = 1000*60; // 1 minute
    private PowerManager mPowerManager;
    //@@@ yujie add for [COOLMAX-912] end
    @Override
    public void onCreate() {
        mVRService = VRManager.getInstance(getBaseContext());

        String device = mVRService.getProperty("ro.product.device");
        //@@@ yujie add for [COOLMAX-912] begin
        mPowerManager = (PowerManager)getBaseContext().getSystemService(Context.POWER_SERVICE);
        //@@@ yujie add for [COOLMAX-912] end
        if ("LeVR_Max_1".equals(device)) {
            VRAdminApplication.setHoldWakeLock(true);
            mPSensorUtility = new PSensorUtility(this);
            mPSensorUtility.registerSensor(new OnPSensorChangeListener() {
                @Override
                public void onNear() {
                    Log.d(TAG, " onNear ");
                    Intent intent = new Intent("android.intent.action.proximity_sensor2");
                    intent.setType("psensor/1");
                    sendBroadcast(intent);
                }
            @Override
            public void onFar() {
                Log.d(TAG, " onFar ");
                //@@@ yujie add for [COOLMAX-912] begin
                PowerManager.WakeLock wakeLock = mPowerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "***VRAdminService_WakeLock_onFar***");
                //Log.d(TAG, "--->>> acquire PARTIAL wakelock! auto release after 1 minute. ");
                wakeLock.acquire(SLEEP_DELAY); // auto release this wakelock after 1 minute
                //@@@ yujie add for [COOLMAX-912] end
                Intent intent = new Intent("android.intent.action.proximity_sensor2");
                intent.setType("psensor/0");
                sendBroadcast(intent);
            }
            });
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    private final IVRAdminService.Stub mBinder =
    new IVRAdminService.Stub() {
        @Override
        public String startDownLoad(String dfuFilePath, String vid, String pid) throws RemoteException {
            Log.d(TAG, "startDownLoad + dfuFilePath:" + dfuFilePath);
            if (bRunning)
                return "-1";
            bRunning = true;
            Thread tlk = new Thread(new TalkToDfuUtil(VRAdminService.this));
            tlk.start();
            try {
               Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            String cmd = "dfu-util -v -a 0 -s :mass-erase:force";
            cmd += " -d " + vid + ":" + pid;
            cmd += " -D " + dfuFilePath + " 2>&1";
            String result = systemCall(cmd);
            bRunning = false;
            return result;
        }

        public String getBoardMode(String vid, String pid) throws RemoteException {
            return systemCall("dfu-util -U /xxx" + "-d " + vid + ":" + pid);
        }

        public void registerCallback(IVRAdminServiceCallback cb) {
            if (cb != null) {
                synchronized (mCallbacks) {
                    Log.e(TAG, "registerCallback to " + cb.getClass().toString());
                    mCallbacks.register(cb);
                }
            }
        }

        public void unregisterCallback(IVRAdminServiceCallback cb) {
            if(cb != null) {
                synchronized (mCallbacks) {
                    Log.e(TAG, "unregisterCallback to " + cb.getClass().toString());
                    mCallbacks.unregister(cb);
                }
            }
        }

        public void reportApplicationInVR(String pkg, boolean activeInVR) {
            Log.d(TAG, "TODO will remove!");
        }

        // LD add.
        private ArrayList<IVRAdminMonitor> mCbs = new ArrayList<IVRAdminMonitor>();
        public void registerMonitor(final IVRAdminMonitor monitor) throws RemoteException {
            if (monitor == null)
                return;
            synchronized (this) {
                if (mCbs.contains(monitor))
                    return;
                mClientCount++;
                mCbs.add(monitor);
                Log.e(TAG, "registerMonitor mClientCount = " + mClientCount);
                monitor.asBinder().linkToDeath(new IBinder.DeathRecipient() {
                    @Override
                    public void binderDied() {
                        mCbs.remove(monitor);
                        mClientCount--;
                        Log.e(TAG, "binderDied clientCount = " + mClientCount);
                        if (0 == mClientCount) {
                            try {
                                // restore status
                                setProperty(VR_GRALLOC_PROPERTY, "0");
                                setSystemNode(MSM_CMD_AUTOREFRESH, "0");
                            } catch (Exception e) {
                                Log.e(TAG, "Error: " + e);
                            }
                        }
                    }
                }, 0);
            }
        }

        public String getSystemNode(String pathname) {
            if (mVRService == null || pathname == null)
                return "";
            return mVRService.getSystemNode(pathname);
        }

        public int setSystemNode(String pathname, String freq) {
            if (mVRService == null || pathname == null)
                return -1;
             return mVRService.setSystemNode(pathname, freq);
        }

        public String systemCall(String cmd) {
            Log.d(TAG, "systemCall");
            if (mVRService == null || cmd == null)
                return "-1";
            return mVRService.systemCall(cmd);
        }

        public String getProperty(String key) {
            if (mVRService == null || key == null)
                return "";
            return mVRService.getProperty(key);
        }

        public int setProperty(String key, String value) {
            if (mVRService == null || key == null)
                return -1;
            return mVRService.setProperty(key, value);
        }

        public int setThreadScheduler(int tid, int policy, int pri) {
            if (mVRService == null)
                return -1;
            return mVRService.setThreadScheduler(tid, policy, pri);
        }

        public int activityEnter() {
            setProperty(VR_GRALLOC_PROPERTY, "1");
            setSystemNode(MSM_CMD_AUTOREFRESH, "1");
            return 0;
        }

        public int activityLeave() {
            setProperty(VR_GRALLOC_PROPERTY, "0");
            setSystemNode(MSM_CMD_AUTOREFRESH, "0");
            return 0;
        }
        // LD end.
    };

    final RemoteCallbackList<IVRAdminServiceCallback> mCallbacks =
        new RemoteCallbackList<IVRAdminServiceCallback> ();

    /**
     * Broadcast the results from the query to all registered callback
     * objects.
     */
    public void broadcastFlashProgress (int percent) {
        // reset the state.
        synchronized (mCallbacks) {
            // Make the calls to all the registered callbacks.
            for (int i = (mCallbacks.beginBroadcast() - 1); i >= 0; i--) {
                IVRAdminServiceCallback cb = mCallbacks.getBroadcastItem(i);
                Log.e(TAG, "broadcasting progress to " + cb.getClass().toString());
                try {
                    cb.onUpgradingUpdate(percent);
                } catch (RemoteException e) {
                }
            }

            // finish up.
            mCallbacks.finishBroadcast();
        }
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onTerminate");
        super.onDestroy();
        if (mPSensorUtility != null) {
            mPSensorUtility.unregisterSensor();
            mPSensorUtility = null;
        }
    }
}

