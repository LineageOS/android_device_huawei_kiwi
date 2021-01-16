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

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.SystemClock;
import android.util.Log;

public class SensorsDozeService extends Service {

    public static final boolean DEBUG = false;
    public static final String TAG = "SensorsDozeService";

    private static final String DOZE_INTENT = "com.android.systemui.doze.pulse";

    private static final int HANDWAVE_DELTA_NS = 1000 * 1000 * 1000;
    private static final int PULSE_MIN_INTERVAL_MS = 5000;
    private static final int SENSORS_WAKELOCK_DURATION = 1000;

    private Context mContext;
    private OrientationSensor mOrientationSensor;
    private PickUpSensor mPickUpSensor;
    private PowerManager mPowerManager;
    private ProximitySensor mProximitySensor;
    private WakeLock mSensorsWakeLock;

    private boolean mDozeEnabled;
    private boolean mHandwaveDoze;
    private boolean mHandwaveGestureEnabled;
    private boolean mPickUpDoze;
    private boolean mPickUpGestureEnabled;
    private boolean mPickUpState;
    private boolean mPocketDoze;
    private boolean mPocketGestureEnabled;
    private boolean mProximityNear;
    private long mLastPulseTimestamp;
    private long mLastStowedTimestamp;

    private OrientationSensor.OrientationListener mOrientationListener =
            new OrientationSensor.OrientationListener() {
        public void onEvent() {
            setOrientationSensor(false);
            handleOrientation();
        }
    };

    private PickUpSensor.PickUpListener mPickUpListener = new PickUpSensor.PickUpListener() {
        public void onEvent() {
            mPickUpState = mPickUpSensor.isPickedUp();
            handlePickUp();
        }
        public void onInit() {
            mPickUpState = mPickUpSensor.isPickedUp();
            if (DEBUG) Log.d(TAG, "Pick-up sensor init : " + mPickUpState);
        }
    };

    private ProximitySensor.ProximityListener mProximityListener =
            new ProximitySensor.ProximityListener() {
        public void onEvent(boolean isNear, long timestamp) {
            mProximityNear = isNear;
            handleProximity(timestamp);
        }
        public void onInit(boolean isNear, long timestamp) {
            if (DEBUG) Log.d(TAG, "Proximity sensor init : " + isNear);
            mLastStowedTimestamp = timestamp;
            mProximityNear = isNear;

            // Pick-up or Orientation sensor initialization
            if (!isEventPending() && !isNear && Utils.isPickUpEnabled(mContext)) {
                setPickUpSensor(true);
            }
        }
    };

    public void onCreate() {
        if (DEBUG) Log.d(TAG, "Creating service");

        super.onCreate();
        mContext = this;

        mPowerManager = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        mSensorsWakeLock = mPowerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                TAG + "WakeLock");
        mOrientationSensor = new OrientationSensor(mContext, mOrientationListener);
        mPickUpSensor = new PickUpSensor(mContext, mPickUpListener);
        mProximitySensor = new ProximitySensor(mContext, mProximityListener);

        IntentFilter intentScreen = new IntentFilter(Intent.ACTION_SCREEN_ON);
        intentScreen.addAction(Intent.ACTION_SCREEN_OFF);
        mContext.registerReceiver(mScreenStateReceiver, intentScreen);
        if (!mPowerManager.isInteractive()) {
            onDisplayOff();
        }
    }

    public int onStartCommand(Intent intent, int flags, int startId) {
        if (DEBUG) Log.d(TAG, "Starting service");
        return START_STICKY;
    }

    public void onDestroy() {
        if (DEBUG) Log.d(TAG, "Destroying service");

        super.onDestroy();
        mContext.unregisterReceiver(mScreenStateReceiver);
        resetSensors();
        setOrientationSensor(false);
        setPickUpSensor(false);
        setProximitySensor(false);
    }

    public IBinder onBind(Intent intent) {
        return null;
    }

    private boolean isEventPending() {
        return mHandwaveDoze || mPickUpDoze || mPocketDoze;
    }

    private void handleProximity(long timestamp) {
        long delta = timestamp - mLastStowedTimestamp;
        boolean quickWave = delta < HANDWAVE_DELTA_NS;
        if (DEBUG) Log.d(TAG, "Proximity sensor : isNear " + mProximityNear);

        // Proximity sensor released
        if (!mProximityNear) {
            mHandwaveDoze = false;
            mPickUpDoze = false;
            mPocketDoze = false;

            boolean isHandwaveEnabled = Utils.isHandwaveGestureEnabled(mContext);
            boolean isPickUpEnabled = Utils.isPickUpEnabled(mContext);
            boolean isPocketEnabled = Utils.isPocketGestureEnabled(mContext);

            // Handwave / Pick-up / Pocket gestures activated
            if (isHandwaveEnabled && isPickUpEnabled && isPocketEnabled) {
                mHandwaveDoze = quickWave;
                mPickUpDoze = !quickWave;
                mPocketDoze = !quickWave;
                setOrientationSensor(true);
            }
            // Handwave Doze detected
            else if (isHandwaveEnabled && quickWave) {
                mHandwaveDoze = true;
                setOrientationSensor(true);
            }
            // Pick-up / Pocket Doze detected
            else if ((isPickUpEnabled || isPocketEnabled) && !quickWave) {
                mPickUpDoze = isPickUpEnabled;
                mPocketDoze = isPocketEnabled;
                setOrientationSensor(true);
            }
            // Start the pick-up sensor
            else if (isPickUpEnabled) {
                setPickUpSensor(true);
            }
        }
        // Proximity sensor stowed
        else {
            mLastStowedTimestamp = timestamp;
            setOrientationSensor(false);
            setPickUpSensor(false);
        }
    }

    private void handleOrientation() {
        if (DEBUG) Log.d(TAG, "Orientation sensor : " + 
                    "FaceDown " + mOrientationSensor.isFaceDown() +
                    ", FaceUp " + mOrientationSensor.isFaceUp() +
                    ", Vertical " + mOrientationSensor.isVertical());

        // Orientation Doze analysis
        if (!mProximityNear) {
            analyseDoze();
        }
    }

    private void handlePickUp() {
        if (DEBUG) Log.d(TAG, "Pick-up sensor : " + mPickUpState);

        // Pick-up Doze analysis
        if (mPickUpState && Utils.isPickUpEnabled(mContext)) {
            mPickUpDoze = true;
            launchWakeLock();
            analyseDoze();
        }
        // Picked-down
        else {
            mPickUpDoze = false;
        }
    }

    private void analyseDoze() {
        if (DEBUG)
            Log.d(TAG, "Doze analysis : HandwaveDoze " + mHandwaveDoze +
                    ", PickUpDoze " + mPickUpDoze +
                    ", PocketDoze " + mPocketDoze +
                    ", PickUpState " + mPickUpState);

        // Handwave Doze launch
        if (mHandwaveDoze && !mOrientationSensor.isFaceDown()) {
            launchDozePulse();
        }
        // Pickup Doze launch
        else if (mPickUpDoze &&
                ((mPickUpState && !mProximityNear) ||
                (!mPickUpState && mOrientationSensor.isFaceDown()))) {
            launchDozePulse();
        }
        // Pocket Doze launch
        else if (mPocketDoze && mOrientationSensor.isVertical()) {
            launchDozePulse();
        }

        // Restore the pick-up sensor
        if (!mProximityNear && Utils.isPickUpEnabled(mContext)) {
            setPickUpSensor(true);
        }

        resetValues();
    }

    private void launchDozePulse() {
        long delta;
        if (mLastPulseTimestamp != 0) {
            delta = SystemClock.elapsedRealtime() - mLastPulseTimestamp;
        } else {
            delta = PULSE_MIN_INTERVAL_MS;
        }

        if (delta >= PULSE_MIN_INTERVAL_MS) {
            if (DEBUG) Log.d(TAG, "Doze launch. Time since last : " + delta);

            launchWakeLock();
            mLastPulseTimestamp = SystemClock.elapsedRealtime();
            Utils.launchDozePulse(mContext);
        }
        else if (DEBUG) Log.d(TAG, "Doze avoided. Time since last : " + delta);
    }

    private void launchWakeLock() {
        mSensorsWakeLock.acquire(SENSORS_WAKELOCK_DURATION);
    }

    private void resetValues() {
        mHandwaveDoze = false;
        mPickUpDoze = false;
        mPocketDoze = false;
    }

    private void setOrientationSensor(boolean enabled) {
        if (enabled) {
            setPickUpSensor(false);
            launchWakeLock();
        }
        mOrientationSensor.setEnabled(enabled);
    }

    private void setPickUpSensor(boolean enabled) {
        if (enabled) {
            setOrientationSensor(false);
        }
        mPickUpSensor.setEnabled(enabled);
    }

    private void setProximitySensor(boolean enabled) {
        mProximitySensor.setEnabled(enabled);
    }

    private void resetSensors() {
        mOrientationSensor.reset();
        mPickUpSensor.reset();
        mProximitySensor.reset();
    }

    private void onDisplayOn() {
        if (DEBUG) Log.d(TAG, "Display on");

        resetSensors();
        setOrientationSensor(false);
        setPickUpSensor(false);
        setProximitySensor(false);
    }

    private void onDisplayOff() {
        if (DEBUG) Log.d(TAG, "Display off");

        mLastPulseTimestamp = 0;
        if (Utils.sensorsEnabled(this)) {
            resetValues();
            resetSensors();
            setOrientationSensor(false);
            setPickUpSensor(false);
            setProximitySensor(true);
        }
    }

    private BroadcastReceiver mScreenStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF)) {
                onDisplayOff();
            } else if (intent.getAction().equals(Intent.ACTION_SCREEN_ON)) {
                onDisplayOn();
            }
        }
    };
}
