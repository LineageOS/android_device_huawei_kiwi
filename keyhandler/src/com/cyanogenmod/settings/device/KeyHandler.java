/*
 * Copyright (C) 2016 The CyanogenMod Project
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

package com.cyanogenmod.settings.device;

import android.Manifest;
import android.content.ActivityNotFoundException;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.res.Resources;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.SystemClock;
import android.os.UserHandle;
import android.os.Vibrator;
import android.provider.Settings;
import android.provider.Settings.Secure;
import android.util.Log;
import android.view.KeyEvent;

import com.android.internal.os.DeviceKeyHandler;
import com.android.internal.util.ArrayUtils;

import cyanogenmod.providers.CMSettings;

import java.util.List;

public class KeyHandler implements DeviceKeyHandler {

    private static final String TAG = KeyHandler.class.getSimpleName();
    private static final int GESTURE_REQUEST = 1;

    /*
     * Supported scancodes:
     * C: launch camera app
     * e: launch E-Mail app
     * M: launch messaging app
     * W: launch browser app
     */
    private static final int KEY_GESTURE_C = 66;
    private static final int KEY_GESTURE_E = 67;
    private static final int KEY_GESTURE_M = 68;
    private static final int KEY_GESTURE_W = 87;

    private static final int GESTURE_WAKELOCK_DURATION = 3000;

    private static final int[] sSupportedGestures = new int[] {
        KEY_GESTURE_C,
        KEY_GESTURE_E,
        KEY_GESTURE_M,
        KEY_GESTURE_W
    };

    private final Context mContext;
    private final PowerManager mPowerManager;
    private EventHandler mEventHandler;
    private SensorManager mSensorManager;
    private Sensor mProximitySensor;
    private Vibrator mVibrator;
    WakeLock mProximityWakeLock;
    WakeLock mGestureWakeLock;
    private int mProximityTimeOut;
    private boolean mProximityWakeSupported;
    private ContentResolver mContentResolver;

    public KeyHandler(Context context) {
        mContext = context;
        mContentResolver = mContext.getContentResolver();
        mPowerManager = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        mEventHandler = new EventHandler();
        mGestureWakeLock = mPowerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                "GestureWakeLock");

        final Resources resources = mContext.getResources();
        mProximityTimeOut = resources.getInteger(
                org.cyanogenmod.platform.internal.R.integer.config_proximityCheckTimeout);
        mProximityWakeSupported = resources.getBoolean(
                org.cyanogenmod.platform.internal.R.bool.config_proximityCheckOnWake);

        if (mProximityWakeSupported) {
            mSensorManager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
            mProximitySensor = mSensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);
            mProximityWakeLock = mPowerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                    "ProximityWakeLock");
        }

        mVibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
        if (mVibrator == null || !mVibrator.hasVibrator()) {
            mVibrator = null;
        }
    }

    private class EventHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            Intent startIntent = null;
            boolean ignoreKey = false;
            boolean broadcast = false;

            switch (msg.arg1) {
                case KEY_GESTURE_C:
                    startIntent = new Intent(
                            cyanogenmod.content.Intent.ACTION_SCREEN_CAMERA_GESTURE);
                    broadcast = true;
                    break;

                case KEY_GESTURE_E:
                    startIntent = getLaunchableIntent(
                            new Intent(Intent.ACTION_VIEW, Uri.parse("mailto:")));
                    break;

                case KEY_GESTURE_M:
                    String defaultApplication = Settings.Secure.getString(mContentResolver,
                                Settings.Secure.SMS_DEFAULT_APPLICATION);
                    PackageManager pm = mContext.getPackageManager();
                    startIntent = pm.getLaunchIntentForPackage(defaultApplication);
                    break;

                case KEY_GESTURE_W:
                    startIntent = getLaunchableIntent(
                            new Intent(Intent.ACTION_VIEW, Uri.parse("http:")));
                    break;

                default:
                    ignoreKey = true;
            }

            if (!ignoreKey) {
                wakeUpAndStartIntent(startIntent, broadcast);
            }
        }
    }

    public boolean handleKeyEvent(KeyEvent event) {
        boolean isKeySupported = ArrayUtils.contains(sSupportedGestures, event.getScanCode());
        if (!isKeySupported) {
            return false;
        }

        if (!mEventHandler.hasMessages(GESTURE_REQUEST)) {
            Message msg = getMessageForKeyEvent(event.getScanCode());
            boolean defaultProximity = isProximityDefaultEnabled();
            boolean proximityWakeCheckEnabled = CMSettings.System.getInt(mContentResolver,
                    CMSettings.System.PROXIMITY_ON_WAKE, defaultProximity ? 1 : 0) == 1;
            if (mProximityWakeSupported && proximityWakeCheckEnabled && mProximitySensor != null) {
                mEventHandler.sendMessageDelayed(msg, mProximityTimeOut);
                processEvent(event.getScanCode());
            } else {
                mEventHandler.sendMessage(msg);
            }
        }
        return true;
    }

    private Message getMessageForKeyEvent(int scancode) {
        Message msg = mEventHandler.obtainMessage(GESTURE_REQUEST);
        msg.arg1 = scancode;
        return msg;
    }

    private void processEvent(final int scancode) {
        mProximityWakeLock.acquire();
        mSensorManager.registerListener(new SensorEventListener() {
            @Override
            public void onSensorChanged(SensorEvent event) {
                mProximityWakeLock.release();
                mSensorManager.unregisterListener(this);
                if (!mEventHandler.hasMessages(GESTURE_REQUEST)) {
                    // The sensor took too long, ignoring.
                    return;
                }
                mEventHandler.removeMessages(GESTURE_REQUEST);
                if (event.values[0] == mProximitySensor.getMaximumRange()) {
                    Message msg = getMessageForKeyEvent(scancode);
                    mEventHandler.sendMessage(msg);
                }
            }

            @Override
            public void onAccuracyChanged(Sensor sensor, int accuracy) {
                // do nothing
            }
        }, mProximitySensor, SensorManager.SENSOR_DELAY_FASTEST);
    }

    private void wakeUpAndStartIntent(Intent intent, boolean broadcast) {
        mGestureWakeLock.acquire(GESTURE_WAKELOCK_DURATION);

        if (broadcast) {
            mContext.sendBroadcast(intent, Manifest.permission.STATUS_BAR_SERVICE);
        } else {
            mPowerManager.wakeUp(SystemClock.uptimeMillis(), "wakeup-gesture");
            startActivitySafely(intent);
        }

        doHapticFeedback();
    }

    private void startActivitySafely(Intent intent) {
        if (intent == null) {
            return;
        }

        intent.addFlags(
                Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_SINGLE_TOP
                | Intent.FLAG_ACTIVITY_CLEAR_TOP);
        try {
            UserHandle user = new UserHandle(UserHandle.USER_CURRENT);
            mContext.startActivityAsUser(intent, null, user);
        } catch (ActivityNotFoundException e) {
            // Ignore
        }
    }

    private void doHapticFeedback() {
        if (mVibrator == null) {
            return;
        }
        boolean enabled = CMSettings.System.getInt(mContentResolver,
                CMSettings.System.TOUCHSCREEN_GESTURE_HAPTIC_FEEDBACK, 1) != 0;
        if (enabled) {
            mVibrator.vibrate(50);
        }
    }

    private boolean isProximityDefaultEnabled() {
        return mContext.getResources().getBoolean(
            org.cyanogenmod.platform.internal.R.bool.config_proximityCheckOnWakeEnabledByDefault);
    }

    private Intent getLaunchableIntent(Intent intent) {
        Intent returnIntent = null;
        PackageManager pm = mContext.getPackageManager();

        List<ResolveInfo> resInfo = pm.queryIntentActivities(intent, 0);
        if (resInfo.size() > 0) {
            ResolveInfo ri = resInfo.get(0);
            returnIntent = pm.getLaunchIntentForPackage(ri.activityInfo.packageName);
        }

        return returnIntent;
    }
}
