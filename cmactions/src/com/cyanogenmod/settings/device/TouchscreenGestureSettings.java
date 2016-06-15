/*
 * Copyright (C) 2015-2016 The CyanogenMod Project
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

import android.app.ActionBar;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.SwitchPreference;
import android.provider.Settings;
import android.view.MenuItem;
import android.view.ViewGroup;

import cyanogenmod.providers.CMSettings;

public class TouchscreenGestureSettings extends PreferenceActivity {

    private static final String KEY_HAPTIC_FEEDBACK = "touchscreen_gesture_haptic_feedback";

    private Handler mGestureHandler = new Handler();
    private SwitchPreference mHapticFeedback;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.touchscreen_panel);
        mHapticFeedback = (SwitchPreference) findPreference(KEY_HAPTIC_FEEDBACK);
        mHapticFeedback.setOnPreferenceChangeListener(mHapticPrefListener);

        for (String gestureKey : CMActionsSettings.ALL_GESTURE_KEYS) {
            Preference pref = findPreference(gestureKey);
            pref.setOnPreferenceChangeListener(mGesturePrefListener);
        }

        final ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);

        ((ViewGroup)getListView().getParent()).setPadding(0, 0, 0, 0);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mHapticFeedback.setChecked(CMSettings.System.getInt(getContentResolver(),
                CMSettings.System.TOUCHSCREEN_GESTURE_HAPTIC_FEEDBACK, 1) != 0);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            onBackPressed();
            return true;
        }
        return false;
    }

    private Preference.OnPreferenceChangeListener mHapticPrefListener =
        new Preference.OnPreferenceChangeListener() {
        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            final boolean value = (Boolean) newValue;
            CMSettings.System.putInt(getContentResolver(),
                    CMSettings.System.TOUCHSCREEN_GESTURE_HAPTIC_FEEDBACK, value ? 1 : 0);
            return true;
        }
    };

    private Preference.OnPreferenceChangeListener mGesturePrefListener =
        new Preference.OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object newValue) {
                mGestureHandler.post(mUpdateGestures);
                return true;
            }
        };

    private final Runnable mUpdateGestures = new Runnable() {
        @Override
        public void run() {
            CMActionsSettings.updateGestureMode(TouchscreenGestureSettings.this);
        }
    };
}
