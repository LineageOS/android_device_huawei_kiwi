/*
 * Copyright (C) 2015 The CyanogenMod Project
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

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.util.Log;

import cyanogenmod.providers.CMSettings;
import org.cyanogenmod.internal.util.FileUtils;

public final class CMActionsSettings {
    private static final String TAG = "CMActions";

    // Proc nodes
    private static final String TOUCHSCREEN_GESTURE_MODE_NODE =
            "/sys/touch_screen/easy_wakeup_gesture";
    private static final String FP_GESTURE_NODE =
            "/sys/devices/platform/fingerprint/diag/navigation_enable";

    // Preference keys
    public static final String[] ALL_GESTURE_KEYS = {
            "touchscreen_gesture_c",
            "touchscreen_gesture_e",
            "touchscreen_gesture_m",
            "touchscreen_gesture_w"
    };

    // Key Masks
    // must be matching the order and KeyCodes of ALL_GESTURE_KEYS
    private static int[] ALL_GESTURE_MASKS = {
            0x080,  // Gesture C
            0x100,  // Gesture E
            0x200,  // Gesture M
            0x400   // Gesture W
    };

    public static final String FP_GESTURE_TAP = "fp_gesture_tap";
    public static final String FP_GESTURE_DOUBLE_TAP = "fp_gesture_double_tap";
    public static final String FP_GESTURE_HOLD = "fp_gesture_hold";
    public static final String FP_GESTURE_UP = "fp_gesture_up";
    public static final String FP_GESTURE_DOWN = "fp_gesture_down";

    public static final String[] ALL_FP_KEYS = {
            FP_GESTURE_TAP,
            FP_GESTURE_DOUBLE_TAP,
            FP_GESTURE_HOLD,
            FP_GESTURE_UP,
            FP_GESTURE_DOWN
    };

    private CMActionsSettings() {
        // this class is not supposed to be instantiated
    }

    /* Use bitwise logic to set gesture_mode in kernel driver */
    public static void updateGestureMode(Context context) {
        boolean doubleTapWakeEnabled = Settings.Secure.getInt(context.getContentResolver(),
                Settings.Secure.DOUBLE_TAP_TO_WAKE, 0) != 0;
        int gestureMode = (doubleTapWakeEnabled ? 1 : 0);

        // Make sure both arrays are set up correctly
        if (ALL_GESTURE_KEYS.length != ALL_GESTURE_MASKS.length) {
            Log.w(TAG, "Array lengths do not match!");
            return;
        }

        SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(context);
        for (int i = 0; i < ALL_GESTURE_KEYS.length; i++) {
            if (sharedPrefs.getBoolean(ALL_GESTURE_KEYS[i], false)) {
                gestureMode |= ALL_GESTURE_MASKS[i];
            }
        }

        Log.d(TAG, "finished gesture mode: " + gestureMode);
        FileUtils.writeLine(TOUCHSCREEN_GESTURE_MODE_NODE, String.valueOf(gestureMode));
    }

    public static boolean isFpGestureEnabled(Context context, String key) {
        return CMSettings.Global.getInt(context.getContentResolver(), key, 0) != 0;
    }

    public static boolean areFpGesturesEnabled(Context context) {
        for (int i = 0; i < ALL_FP_KEYS.length; i++) {
            if (isFpGestureEnabled(context, ALL_FP_KEYS[i])) {
                return true;
            }
        }

        return false;
    }

    public static void enableFpGestures(Context context) {
        FileUtils.writeLine(FP_GESTURE_NODE, areFpGesturesEnabled(context) ? "1" : "0");
    }
}
