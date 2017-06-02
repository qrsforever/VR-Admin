package com.android.VRAdmin;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.os.IBinder;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.SystemClock;
import android.util.Log;

import android.os.SystemProperties;

public class HMDReceiver extends BroadcastReceiver {

    private static final String TAG = "HMDReceiver";
    private static final int VR_HMD_VENDOR_ID = 0x2b0e;
    private static final int VR_HMD_PRODUCT_ID = 0x1bfb;

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.d(TAG, "onReceive : " + action);
        UsbDevice usbDevice = (UsbDevice) intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
        if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
            if (usbDevice != null) {
                // TODO device_filter is invalid
                Log.d(TAG, "Usb device:vid:0x" + Integer.toString(usbDevice.getVendorId(), 16) +
                      " pid:0x" + Integer.toString(usbDevice.getProductId(), 16));
                if (isPhonePlugin(usbDevice)) {
                    Log.d(TAG, "VR HMD in");
                    Intent intentSend = new Intent("android.intent.action.lvr_hmd_in");
                    context.sendBroadcast(intentSend);
                    VRAdminApplication.onUSBAttached();
                }
            }
        } else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
            if (usbDevice != null) {
                // TODO device_filter is invalid
                Log.d(TAG, "Usb device:vid:0x" + Integer.toString(usbDevice.getVendorId(), 16) +
                      " pid:0x" + Integer.toString(usbDevice.getProductId(), 16));
                if (isPhonePlugin(usbDevice)) {
                    Log.d(TAG, "VR HMD out");
                    Intent intentSend = new Intent("android.intent.action.lvr_hmd_out");
                    context.sendBroadcast(intentSend);
                    VRAdminApplication.onUSBDetached();
                }
            }
        } else if (action.equals("android.intent.action.BOOT_COMPLETED")) {
            Log.d(TAG, "Get BOOT_COMPLETED!");
            Intent i = new Intent(context, VRAdminService.class);
            context.startService(i);
        }
        if (action.equals("android.intent.action.proximity_sensor2")) {
            String type = intent.getType();
            Log.d(TAG, "VR HMD proximity_sensor:" + type);
            if (type.equals("psensor/1")) {
                VRAdminApplication.onPSensorNear();
            } else if (type.equals("psensor/0")) {
                VRAdminApplication.onPSensorFar();
            }
        }
    }

    private boolean isPhonePlugin(UsbDevice device) {
        int productid = device.getProductId();
        int vendorid = device.getVendorId();
        if (vendorid == VR_HMD_VENDOR_ID && productid == VR_HMD_PRODUCT_ID) {
            return true;
        }
        return false;
    }

}
