package com.android.VRAdmin;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;

import android.app.Service;
import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.util.Log;

public class TalkToDfuUtil implements Runnable {
    private static final String TAG = "TalkToDfuUtil";
    private VRAdminService mService = null;
    private LocalServerSocket mVrSocket = null;
    private LocalSocket mCliSocket = null;

    public TalkToDfuUtil(VRAdminService service) {
        mService = service;
    }

    private void talkWithSocket() {
        int progress = 0;
        int fileSize = -1;
        int writeSize = 0;
        try {
            mVrSocket = new LocalServerSocket("com.le.vrotalib");
            mCliSocket = mVrSocket.accept(); //TODO block, need recoding...
            if (mCliSocket != null) {
                Log.e(TAG, "TalkToDfuUtil accept OK");
                InputStream is = mCliSocket.getInputStream();
                while (true) {
                    byte[] buffer = new byte[4];
                    buffer[0] = 0;
                    buffer[1] = 0;
                    int count = is.read(buffer);
                    Log.e(TAG, "count:" + count);
                    for (int i = 0; i < 4; i++) {
                        Log.e(TAG, String.format("buffer[%d]:%s", i, Integer.toHexString(buffer[i] & 0xff)));
                    }
                    int value = (buffer[0] & 0xff) | ((buffer[1] & 0xff) << 8) |
                        ((buffer[2] & 0xff) << 16) | ((buffer[3] & 0xff) << 24);
                    Log.e(TAG, "value:" + value + " hex:" + Integer.toHexString(value));

                    if (count < 0) {
                        Log.e(TAG, "server read error");
                        break;
                    } else if (count == 0) {
                        // clientSock is closed by client
                        Log.e(TAG, "client exit");
                        break;
                    }

                    if (fileSize == -1) {
                        fileSize = value;
                    } else {
                        writeSize = value;
                    }
                    progress = writeSize * 100 / fileSize;
                    Log.e(TAG, String.format("fileSize:%d writeSize:%d progress:%d", fileSize, writeSize, progress));
                    if (mService != null) {
                        Log.e(TAG, "mService.broadcastFlashProgress:" + progress);
                        mService.broadcastFlashProgress(progress);
                    }
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (fileSize != writeSize && mService != null) {
                Log.e(TAG, "dfutil error, dfu-util quit!");
                mService.broadcastFlashProgress(-1);
            }
            try {
                if (mCliSocket != null) {
                    mCliSocket.close();
                    mCliSocket = null;
                }
                if (mVrSocket != null) {
                    mVrSocket.close();
                    mVrSocket = null;
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
            Log.d(TAG, "TalkToDfuUtil End!");
        }
    }

    @Override
    public void run() {
        talkWithSocket();
    }
}
