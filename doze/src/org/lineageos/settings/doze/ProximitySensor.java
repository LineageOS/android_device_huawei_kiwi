/*
 * Copyright (c) 2015 The CyanogenMod Project
 * Copyright (c) 2017-2021 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.lineageos.settings.doze;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

public class ProximitySensor implements SensorEventListener {

    private static final int PROXIMITY_DELAY = 1000 * 1000;
    private static final int PROXIMITY_LATENCY = 100 * 1000;

    private boolean mEnabled;
    private boolean mReady;
    private boolean mState;
    private float mMaxRange;
    private ProximityListener mProximityListener;
    private Sensor mProximitySensor;
    private SensorManager mSensorManager;
    private ExecutorService mExecutorService;

    public interface ProximityListener {
        void onEvent(boolean isNear, long timestamp);
        void onInit(boolean isNear, long timestamp);
    }

    public ProximitySensor(Context context, ProximityListener proximitylistener) {
        mSensorManager = context.getSystemService(SensorManager.class);
        mProximitySensor = mSensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY, true);

        mProximityListener = proximitylistener;
        if (mProximitySensor != null) {
            mMaxRange = mProximitySensor.getMaximumRange();
        }

        mExecutorService = Executors.newSingleThreadExecutor();
    }

    public void onAccuracyChanged(Sensor sensor, int accuracy) { }

    public void onSensorChanged(SensorEvent event) {
        if (event.values.length == 0) return;
        boolean isNear = (event.values[0] < mMaxRange);

        // Launch an event
        if (mState != isNear) {
            mState = isNear;
            if (mReady) {
                mProximityListener.onEvent(mState, event.timestamp);
            }
        }

        // Init the sensor
        if (!mReady) {
            mProximityListener.onInit(mState, event.timestamp);
            mReady = true;
        }
    }

    public void setEnabled(final boolean enabled) {
        if (enabled == mEnabled || mProximitySensor == null) {
            return;
        }
        reset();
        submit(() -> {
            if (enabled) {
                mSensorManager.registerListener(this, mProximitySensor, PROXIMITY_DELAY,
                        PROXIMITY_LATENCY);
            } else {
                mSensorManager.unregisterListener(this, mProximitySensor);
            }
            mEnabled = enabled;
        });
    }

    public void reset() {
        mReady = false;
        mState = false;
    }

    private Future<?> submit(Runnable runnable) {
        return mExecutorService.submit(runnable);
    }
}
