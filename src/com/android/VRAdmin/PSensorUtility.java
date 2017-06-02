package com.android.VRAdmin;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.util.Log;

public class PSensorUtility {

    private static final String TAG = "PSensorUtility";
    private SensorEventListener mSensorListener = null;
    private SensorManager mSensorMgr = null;
    private Sensor mPSensor = null;
    private boolean isListening = false;
    private float mMaxiRange = 3.5f;

    PSensorUtility(Context context) {
        mSensorMgr = (SensorManager)context.getSystemService(Context.SENSOR_SERVICE);
        mPSensor = mSensorMgr.getDefaultSensor(Sensor.TYPE_PROXIMITY);
        // mMaxiRange = mPSensor.getMaximumRange();
    }

    public void registerSensor(final OnPSensorChangeListener listener) {
        if (isListening) {
            return;
        }
        isListening = true;

        Log.d(TAG, "registerSensor");

        if (mSensorMgr == null || mPSensor == null) {
            Log.e(TAG, "check support ?");
            return;
        }

        mSensorListener = new SensorEventListener() {

            @Override
            public void onSensorChanged(SensorEvent event) {
                if (event.sensor.getType() == Sensor.TYPE_PROXIMITY) {
                    Log.d(TAG, "proximity_sensor: " + event.values[0]);
                    float value = event.values[0];
                    if (value < mMaxiRange) {
                        listener.onNear();
                    } else {
                        listener.onFar();
                    }
                }
            }

            @Override
            public void onAccuracyChanged(Sensor sensor, int accuracy) {
            }

        };

        if (mSensorListener != null) {
            mSensorMgr.registerListener(mSensorListener, mPSensor, SensorManager.SENSOR_DELAY_NORMAL);
        }
    }

    public void unregisterSensor() {
        if (!isListening) {
            return;
        }
        isListening = false;

        mSensorMgr.unregisterListener(mSensorListener);
        mSensorListener = null;
    }
}
