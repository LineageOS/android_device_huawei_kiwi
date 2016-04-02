/*
 * Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 *
 * Copyright (C) 2006 The Android Open Source Project
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

package com.android.internal.telephony;

import static com.android.internal.telephony.RILConstants.*;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Message;
import android.os.Parcel;
import android.telephony.RadioAccessFamily;
import android.text.TextUtils;


import android.telephony.Rlog;

/**
 * RIL customization for kiwi devices
 *
 * {@hide}
 */
public class KiwiRIL extends RIL {
    private static final int BYTE_SIZE = 1;
    private static final String OEM_IDENTIFIER = "QOEMHOOK";

    /** Starting number for OEMHOOK request and response IDs */
    private static final int OEMHOOK_BASE = 0x80000;

    private static final int OEMHOOK_EVT_HOOK_UPDATE_STACK_BINDING = OEMHOOK_BASE + 24;

    private static final int BIND_TO_STACK = 1;
    private static final int UNBIND_TO_STACK = 0;
    private static final int ACTIVE_STACK = 0;
    private static final int STANDBY_STACK = 1;

    private final RadioCapability mRcActive;
    private final RadioCapability mRcStandby;
    private RadioCapability mRc;
    private final int mOtherInstanceId;

    public KiwiRIL(Context context, int networkMode, int cdmaSubscription, Integer instanceId) {
        super(context, networkMode, cdmaSubscription, instanceId);

        String rafString = mContext.getResources().getString(
                com.android.internal.R.string.config_radio_access_family);
        mRcActive = stringToRadioCapabilities(rafString, instanceId.toString());
        mRcStandby = stringToRadioCapabilities("GSM", instanceId.toString());

        if (instanceId == 0) {
            riljLog("Setting initial rc to active");
            mRc = mRcActive;
            mOtherInstanceId = 1;
        } else {
            riljLog("Setting initial rc to standby");
            mRc = mRcStandby;
            mOtherInstanceId = 0;
        }
    }

    @Override
    public void getRadioCapability(Message response) {
        if (response != null) {
            AsyncResult.forMessage(response, mRc, null);
            response.sendToTarget();
        }
    }

    @Override
    public void setRadioCapability(RadioCapability rc, Message response) {
        riljLog("setRadioCapabilty: newRaf=" + rc.getRadioAccessFamily());
        if (rc.getRadioAccessFamily() == mRc.getRadioAccessFamily()) {
            riljLog("setRadioCapbility: stack unchanged");
        } if (rc.getRadioAccessFamily() == mRcActive.getRadioAccessFamily()) {
            riljLog("setRadioCapbility: setting active raf");
            updateStackBinding(STANDBY_STACK, UNBIND_TO_STACK, null);
            updateStackBinding(ACTIVE_STACK, BIND_TO_STACK, null);
            mRc = rc;
        } else if (rc.getRadioAccessFamily() == mRcStandby.getRadioAccessFamily()) {
            riljLog("setRadioCapbility: setting standby raf");
            updateStackBinding(ACTIVE_STACK, UNBIND_TO_STACK, null);
            updateStackBinding(STANDBY_STACK, BIND_TO_STACK, null);
            mRc = rc;
        } else {
            riljLog("Setting unknown raf " + rc.getRadioAccessFamily());
        }
        if (response != null) {
            AsyncResult.forMessage(response, mRc, null);
            response.sendToTarget();
        }
    }

    @Override
    protected Object
    responseFailCause(Parcel p) {
        int numInts;
        int response[];

        numInts = p.readInt();
        response = new int[numInts];
        for (int i = 0 ; i < numInts ; i++) {
            response[i] = p.readInt();
        }
        LastCallFailCause failCause = new LastCallFailCause();
        failCause.causeCode = response[0];
        if (p.dataAvail() > 0) {
          failCause.vendorCause = p.readString();
        }
        return failCause;
    }

    @Override
    protected void
    send(RILRequest rr) {
        if (rr.mRequest >= 132) {
            Rlog.i(RILJ_LOG_TAG, "KiwiRil: Unsupported request " + rr.mRequest);
            rr.onError(REQUEST_NOT_SUPPORTED, null);
            rr.release();
        } else {
            super.send(rr);
        }
    }

    private RadioCapability stringToRadioCapabilities(String rafString, String uuid) {
        int raf = RadioAccessFamily.RAF_UNKNOWN;

        if (TextUtils.isEmpty(rafString) == false) {
            raf = RadioAccessFamily.rafTypeFromString(rafString);
        }
        return new RadioCapability(mInstanceId.intValue(), 0, 0, raf,
                        uuid, RadioCapability.RC_STATUS_SUCCESS);
    }

    private void updateStackBinding(int stackId, int enable, Message response) {
        byte[] payload = new byte[]{(byte) stackId, (byte) enable};
        Rlog.d(RILJ_LOG_TAG, "updateStackBinding: stackId=" + stackId + " enable=" + enable);

        sendOemRilRequestRaw(OEMHOOK_EVT_HOOK_UPDATE_STACK_BINDING, payload.length, payload, response);
    }

    private void sendOemRilRequestRaw(int requestId, int numPayload, byte[] payload,
            Message response) {
        byte[] request = new byte[mHeaderSize + numPayload * BYTE_SIZE];

        ByteBuffer buf= ByteBuffer.wrap(request);
        buf.order(ByteOrder.nativeOrder());

        // Add OEM identifier String
        buf.put(OEM_IDENTIFIER.getBytes());
        // Add Request ID
        buf.putInt(requestId);
        if (numPayload > 0 && payload != null) {
            // Add Request payload length
            buf.putInt(numPayload * BYTE_SIZE);
            for (byte b : payload) {
                buf.put(b);
            }
        }

        invokeOemRilRequestRaw(request, response);
    }
}
