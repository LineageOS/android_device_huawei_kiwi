/*
 * Copyright (c) 2015 The CyanogenMod Project
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

package org.cyanogenmod.sensors;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;

import java.lang.InterruptedException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

public class ProximityCalibrateActivity extends Activity {

    private static final String TAG = "ProximityCalibrate";
    private static final boolean DEBUG = Log.isLoggable(TAG, Log.DEBUG);

    public static final String EXTRA_SECRET_CODE = "secret_code";

    private static final int PARAM_HI = 0;
    private static final int PARAM_LO = 1;
    private static final int PARAM_CROSSTALK = 2;
    private static final int PARAM_RESULT = 3;
    private static final int PARAM_OFFSET = 4;
    private static final int PARAM_MAX = 5;

    private static final int CROSSTALK_MIN = 0;
    private static final int CROSSTALK_MIDDLE = 600;
    private static final int CROSSTALK_MAX = 1023;

    private static final int MSG_CALIBRATE = 0;
    private static final int MSG_USE_ORIGINAL = 1;

    private static final String BUNDLE_CALIBRATE_PARAMS = "params";

    private TextView mUserActionText;
    private TextView mResultText;
    private TextView mCrosstalkText;
    private TextView mXText;
    private TextView mYText;
    private TextView mOffsetText;
    private Button mStartButton;

    private static final ExecutorService mExecutorService = Executors.newSingleThreadExecutor();
    private UIHandler mHandler = new UIHandler();
    private CalibrateRunnable mCalibrateRunnable = new CalibrateRunnable();
    private Future mFuture;

    /**
     * Setup native bits
     */
    static {
        System.load("libjni_proximityCalibrate.so");
    }

    private static native boolean native_calibrateProximity(int[] parameters);

    /* Lifecycle bits */

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        String code = getIntent().getStringExtra(EXTRA_SECRET_CODE);
        if (TextUtils.isEmpty(code) || !code.equals("7769")) {
            Log.e(TAG, "Did not enter by dialer code!");
            finish();
        }

        getWindow().requestFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        setContentView(R.layout.main);

        mStartButton = (Button) findViewById(R.id.start_btn);
        Button exitButton = (Button) findViewById(R.id.exit_btn);

        mUserActionText = (TextView) findViewById(R.id.action);
        mResultText = (TextView) findViewById(R.id.result_calibrate);
        mCrosstalkText = (TextView) findViewById(R.id.result_crosstalk);
        mXText = (TextView) findViewById(R.id.result_x);
        mYText = (TextView) findViewById(R.id.result_y);
        mOffsetText = (TextView) findViewById(R.id.result_offset);

        mStartButton.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mFuture == null) {
                    start();
                } else {
                    stop();
                }
            }
        });

        exitButton.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });
    }

    private void start() {
        stop(); // Always stop to make sure we start fresh
        mFuture = mExecutorService.submit(mCalibrateRunnable);
        mStartButton.setText("Stop");
    }

    private void stop() {
        mCalibrateRunnable.mRun = false;
        if (mFuture != null) {
            mFuture.cancel(true);
            mFuture = null;
        }
        mStartButton.setText("Start");
    }

    @Override
    protected void onResume() {
        if (DEBUG) {
            Log.d(TAG, "onResume");
        }
        SensorManager sm = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        Sensor proximitySensor = sm.getDefaultSensor(Sensor.TYPE_PROXIMITY);
        sm.registerListener(mSensorEventListener, proximitySensor,
                SensorManager.SENSOR_DELAY_FASTEST);
        super.onResume();
    }

    @Override
    protected void onPause() {
        if (DEBUG) {
            Log.d(TAG, "onPause");
        }
        super.onPause();
        stop();
        SensorManager sm = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        sm.unregisterListener(mSensorEventListener);
        mHandler.removeMessages(MSG_CALIBRATE);
        mHandler.removeMessages(MSG_USE_ORIGINAL);
    }

    private class CalibrateRunnable implements Runnable {
        public boolean mRun = false;

        @Override
        public void run() {
            mRun = true;
            while (mRun) {
                if (DEBUG) {
                    Log.d(TAG, "Looping...");
                }
                int[] parameters = new int[PARAM_MAX];

                parameters[PARAM_RESULT] = -1;
                native_calibrateProximity(parameters);

                Message msg = parameters[PARAM_RESULT] == -1 ?
                        mHandler.obtainMessage(MSG_USE_ORIGINAL) :
                        mHandler.obtainMessage(MSG_CALIBRATE);
                Bundle data = new Bundle();
                data.putIntArray(BUNDLE_CALIBRATE_PARAMS, parameters);
                msg.setData(data);
                mHandler.sendMessage(msg);
                try {
                    Thread.sleep(100);
                } catch (InterruptedException ie) {
                    Log.w(TAG, ie.getMessage(), ie);
                }
            }
        }
    }

    private class UIHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            int[] parameters;
            switch (msg.what) {
                case MSG_USE_ORIGINAL:
                    if (DEBUG) {
                        Log.d(TAG, "Use Original Message Received");
                    }
                    parameters = msg.getData().getIntArray(BUNDLE_CALIBRATE_PARAMS);
                    if (parameters[2] >= CROSSTALK_MIN &&
                            parameters[2] <= CROSSTALK_MIDDLE) {
                        mResultText.setText(R.string.calibration_good);
                        mResultText.setTextColor(Color.GREEN);
                    } else if (parameters[2] > CROSSTALK_MIDDLE &&
                            parameters[2] <= CROSSTALK_MAX) {
                        mResultText.setText(R.string.calibration_inferior);
                        mResultText.setTextColor(Color.RED);
                    } else {
                        mResultText.setText(R.string.calibration_fail);
                        mResultText.setTextColor(Color.RED);
                    }
                    mCrosstalkText.setText("Crosstalk = " + parameters[PARAM_CROSSTALK]);
                    mXText.setText("High = " + parameters[PARAM_HI]);
                    mYText.setText("Low = " + parameters[PARAM_LO]);
                    break;
                case MSG_CALIBRATE:
                    if (DEBUG) {
                        Log.d(TAG, "Calibrate Message Received");
                    }
                    parameters = msg.getData().getIntArray(BUNDLE_CALIBRATE_PARAMS);
                    if (parameters[PARAM_RESULT] == 0) {
                        //not calibration
                        mResultText.setText(R.string.calibration_none);
                        mResultText.setTextColor(Color.RED);
                    } else if (parameters[PARAM_RESULT] == 1) {
                        //well
                        mResultText.setText(R.string.calibration_good);
                        mResultText.setTextColor(Color.GREEN);
                    } else if (parameters[PARAM_RESULT] == 2) {
                        //fail
                        mResultText.setText(R.string.calibration_fail);
                        mResultText.setTextColor(Color.RED);
                    } else if (parameters[PARAM_RESULT] == 3) {
                        //too bright
                        mResultText.setText(R.string.calibration_bright);
                        mResultText.setTextColor(Color.RED);
                    }
                    mCrosstalkText.setText("Crosstalk = " + parameters[PARAM_CROSSTALK]);
                    mXText.setText("High = " + parameters[PARAM_HI]);
                    mYText.setText("Low = " + parameters[PARAM_LO]);
                    mOffsetText.setText("Offset = " + parameters[PARAM_OFFSET]);
                    break;
                default:
                    break;
            }
            super.handleMessage(msg);
        }
    }

    private SensorEventListener mSensorEventListener = new SensorEventListener() {
        @Override
        public void onSensorChanged(SensorEvent event) {
            float[] data = event.values;
            if (DEBUG) {
                Log.d(TAG, "Sensor Value: " + data[0]);
            }
            if (data[0] > 0) {
                mUserActionText.setText("come near");
            } else {
                mUserActionText.setText("move away");
            }
            mUserActionText.setTextColor(Color.YELLOW);
        }

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            /* Empty */
        }
    };
}
