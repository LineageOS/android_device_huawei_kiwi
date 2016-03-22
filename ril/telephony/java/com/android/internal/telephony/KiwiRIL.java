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

import android.content.Context;

import android.telephony.Rlog;

/**
 * RIL customization for kiwi devices
 *
 * {@hide}
 */
public class KiwiRIL extends RIL {
    public KiwiRIL(Context context, int networkMode, int cdmaSubscription, Integer instanceId) {
        super(context, networkMode, cdmaSubscription, instanceId);
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
}
