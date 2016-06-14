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
import android.util.Log;

import com.cyanogenmod.settings.device.utils.FileUtils;

public class CMActionsSettings {
    private static final String TAG = "CMActions";

    // Preference keys
    public static final String TOUCHSCREEN_C_GESTURE_KEY = "touchscreen_gesture_c";
    public static final String TOUCHSCREEN_E_GESTURE_KEY = "touchscreen_gesture_e";
    public static final String TOUCHSCREEN_M_GESTURE_KEY = "touchscreen_gesture_m";
    public static final String TOUCHSCREEN_W_GESTURE_KEY = "touchscreen_gesture_w";

    // Proc nodes
    private static final String TOUCHSCREEN_GESTURE_MODE_NODE =
            "/sys/touch_screen/easy_wakeup_gesture";

    // Key Masks
    private static int KEY_MASK_GESTURE_C = 0x080;
    private static int KEY_MASK_GESTURE_E = 0x100;
    private static int KEY_MASK_GESTURE_M = 0x200;
    private static int KEY_MASK_GESTURE_W = 0x400;

    /* Use bitwise logic to set gesture_mode in kernel driver */
    public static void updateGestureMode(Context context) {
        int gesture_mode = 0;
        boolean mIsCGestureEnabled;
        boolean mIsEGestureEnabled;
        boolean mIsMGestureEnabled;
        boolean mIsWGestureEnabled;

        SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(context);
        mIsCGestureEnabled = sharedPrefs.getBoolean(TOUCHSCREEN_C_GESTURE_KEY, false);
        mIsEGestureEnabled = sharedPrefs.getBoolean(TOUCHSCREEN_E_GESTURE_KEY, false);
        mIsMGestureEnabled = sharedPrefs.getBoolean(TOUCHSCREEN_M_GESTURE_KEY, false);
        mIsWGestureEnabled = sharedPrefs.getBoolean(TOUCHSCREEN_W_GESTURE_KEY, false);

        if (mIsCGestureEnabled)
            gesture_mode = gesture_mode | KEY_MASK_GESTURE_C;
        if (mIsEGestureEnabled)
            gesture_mode = gesture_mode | KEY_MASK_GESTURE_E;
        if (mIsMGestureEnabled)
            gesture_mode = gesture_mode | KEY_MASK_GESTURE_M;
        if (mIsWGestureEnabled)
            gesture_mode = gesture_mode | KEY_MASK_GESTURE_W;

        Log.d(TAG, "finished gesture mode: " + gesture_mode);
        FileUtils.writeLine(TOUCHSCREEN_GESTURE_MODE_NODE, String.valueOf(gesture_mode));
    }
}
