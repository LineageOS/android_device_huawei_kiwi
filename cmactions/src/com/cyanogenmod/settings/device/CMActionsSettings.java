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

import android.content.ContentResolver;
import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.util.Log;

import com.cyanogenmod.settings.device.utils.FileUtils;

import java.lang.Integer;

public class CMActionsSettings {
    private static final String TAG = "CMActions";

    // Preference keys
    private static final String TOUCHSCREEN_C_GESTURE_KEY = "touchscreen_gesture_c";
    private static final String TOUCHSCREEN_E_GESTURE_KEY = "touchscreen_gesture_e";
    private static final String TOUCHSCREEN_M_GESTURE_KEY = "touchscreen_gesture_m";
    private static final String TOUCHSCREEN_W_GESTURE_KEY = "touchscreen_gesture_w";

    // Proc nodes
    public static final String TOUCHSCREEN_GESTURE_MODE_NODE =
            "/sys/touch_screen/easy_wakeup_gesture";

    // Key Masks
    public final int KEY_MASK_GESTURE_C = 0x080;
    public final int KEY_MASK_GESTURE_E = 0x100;
    public final int KEY_MASK_GESTURE_M = 0x200;
    public final int KEY_MASK_GESTURE_W = 0x400;

    private boolean mIsGesture_C_Enabled;
    private boolean mIsGesture_E_Enabled;
    private boolean mIsGesture_M_Enabled;
    private boolean mIsGesture_W_Enabled;

    private final Context mContext;

    public CMActionsSettings(Context context ) {
        SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(context);
        loadPreferences(sharedPrefs);
        sharedPrefs.registerOnSharedPreferenceChangeListener(mPrefListener);
        mContext = context;
    }

    public void loadPreferences(SharedPreferences sharedPreferences) {
        mIsGesture_C_Enabled = sharedPreferences.getBoolean(TOUCHSCREEN_C_GESTURE_KEY, false);
        mIsGesture_E_Enabled = sharedPreferences.getBoolean(TOUCHSCREEN_E_GESTURE_KEY, false);
        mIsGesture_M_Enabled = sharedPreferences.getBoolean(TOUCHSCREEN_M_GESTURE_KEY, false);
        mIsGesture_W_Enabled = sharedPreferences.getBoolean(TOUCHSCREEN_W_GESTURE_KEY, false);
        updateGestureMode();
    }

    private SharedPreferences.OnSharedPreferenceChangeListener mPrefListener =
            new SharedPreferences.OnSharedPreferenceChangeListener() {
                @Override
                public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
                    boolean updated = true;

                    if (TOUCHSCREEN_C_GESTURE_KEY.equals(key)) {
                        mIsGesture_C_Enabled = sharedPreferences.getBoolean(
                                TOUCHSCREEN_C_GESTURE_KEY, false);
                    } else if (TOUCHSCREEN_E_GESTURE_KEY.equals(key)) {
                        mIsGesture_E_Enabled = sharedPreferences.getBoolean(
                                TOUCHSCREEN_E_GESTURE_KEY, false);
                    } else if (TOUCHSCREEN_M_GESTURE_KEY.equals(key)) {
                        mIsGesture_M_Enabled = sharedPreferences.getBoolean(
                                TOUCHSCREEN_M_GESTURE_KEY, false);
                    } else if (TOUCHSCREEN_W_GESTURE_KEY.equals(key)) {
                        mIsGesture_W_Enabled = sharedPreferences.getBoolean(
                                TOUCHSCREEN_W_GESTURE_KEY, false);
                    } else {
                        updated = false;
                    }
                    if (updated) {
                        updateGestureMode();
                    }
                }
            };

    /* Use bitwise logic to set gesture_mode in kernel driver.
       Check each if each key is enabled with & operator and KEY_MASK,
       if enabled toggle the appropriate bit with ^ XOR operator */
    public void updateGestureMode() {
        int gesture_mode = 0;

        if (((gesture_mode & KEY_MASK_GESTURE_C) == 1) != mIsGesture_C_Enabled)
            gesture_mode = (gesture_mode ^ KEY_MASK_GESTURE_C);
        if (((gesture_mode & KEY_MASK_GESTURE_E) == 1) != mIsGesture_E_Enabled)
            gesture_mode = (gesture_mode ^ KEY_MASK_GESTURE_E);
        if (((gesture_mode & KEY_MASK_GESTURE_M) == 1) != mIsGesture_M_Enabled)
            gesture_mode = (gesture_mode ^ KEY_MASK_GESTURE_M);
        if (((gesture_mode & KEY_MASK_GESTURE_W) == 1) != mIsGesture_W_Enabled)
            gesture_mode = (gesture_mode ^ KEY_MASK_GESTURE_W);

        Log.d(TAG, "finished gesture mode: " + gesture_mode);
        FileUtils.writeLine(TOUCHSCREEN_GESTURE_MODE_NODE, String.valueOf(gesture_mode));
    }

}
