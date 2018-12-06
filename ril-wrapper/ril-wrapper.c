/*
 * Copyright (C) 2018 The LineageOS Project
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

#define LOG_TAG "ril-wrapper"

/*
 * We're using RIL_Env, which is only exposed if this is defined.
 */
#define RIL_SHLIB

#include <log/log.h>
#include <telephony/ril.h>

#include <dlfcn.h>

#define RIL_LIB_NAME "libril-qc-qmi-1.so"

typedef struct {
    int rscp;
    int ecio;
} RIL_WCDMA_SignalStrength;

typedef struct {
    RIL_GW_SignalStrength GW_SignalStrength;
    RIL_WCDMA_SignalStrength WCDMA_SignalStrength;
    RIL_CDMA_SignalStrength CDMA_SignalStrength;
    RIL_EVDO_SignalStrength EVDO_SignalStrength;
    RIL_LTE_SignalStrength_v8 LTE_SignalStrength;
    RIL_TD_SCDMA_SignalStrength TD_SCDMA_SignalStrength;
} RIL_SignalStrength_v10_vendor;

/*
 * These structs are only avaiable in ril_internal.h
 * which is not exposed.
 */
typedef struct {
    int requestNumber;
    void (*dispatchFunction)(void* p, void* pRI);
    int (*responseFunction)(void* p, void* response, size_t responselen);
} CommandInfo;

typedef struct RequestInfo {
    int32_t token;
    CommandInfo* pCI;
    struct RequestInfo* p_next;
    char cancelled;
    char local;
} RequestInfo;

static const RIL_RadioFunctions* qmiRilFunctions;
static const struct RIL_Env* ossRilEnv;

static size_t transformResponse(const void* response) {
    RIL_SignalStrength_v10_vendor vendorResponse = *(RIL_SignalStrength_v10_vendor*)response;
    RIL_SignalStrength_v10* rilResponse = (RIL_SignalStrength_v10*)response;
    int gsmSignalStrength = vendorResponse.GW_SignalStrength.signalStrength;

    if (gsmSignalStrength != -1) {
        gsmSignalStrength = -(gsmSignalStrength - 113) / 2;
    }

    rilResponse->GW_SignalStrength.signalStrength = vendorResponse.WCDMA_SignalStrength.rscp <= 0
                                                        ? gsmSignalStrength
                                                        : vendorResponse.WCDMA_SignalStrength.rscp;
    rilResponse->GW_SignalStrength.bitErrorRate = vendorResponse.GW_SignalStrength.bitErrorRate;

    rilResponse->CDMA_SignalStrength = vendorResponse.CDMA_SignalStrength;
    rilResponse->EVDO_SignalStrength = vendorResponse.EVDO_SignalStrength;
    rilResponse->LTE_SignalStrength = vendorResponse.LTE_SignalStrength;
    rilResponse->TD_SCDMA_SignalStrength.rscp = vendorResponse.WCDMA_SignalStrength.rscp;

    return sizeof(RIL_SignalStrength_v10);
}

static void onRequestCompleteShim(RIL_Token t, RIL_Errno e, void* response, size_t responselen) {
    if (!response) {
        ALOGV("%s: response is NULL", __func__);
        goto do_not_handle;
    }

    RequestInfo* requestInfo = (RequestInfo*)t;
    if (!requestInfo) {
        ALOGE("%s: request info is NULL", __func__);
        goto do_not_handle;
    }

    int request = requestInfo->pCI->requestNumber;
    switch (request) {
        case RIL_REQUEST_SIGNAL_STRENGTH:
            if (responselen != sizeof(RIL_SignalStrength_v10_vendor)) {
                ALOGE("%s: invalid response length of RIL_REQUEST_SIGNAL_STRENGTH", __func__);
                goto do_not_handle;
            }

            responselen = transformResponse(response);
            break;

        case RIL_REQUEST_DEVICE_IDENTITY:
            if (responselen != 5 * sizeof(char*)) {
                ALOGE("%s: invalid response length of RIL_REQUEST_DEVICE_IDENTITY", __func__);
                goto do_not_handle;
            }

            responselen = 4 * sizeof(char*);
            break;
    }

do_not_handle:
    ossRilEnv->OnRequestComplete(t, e, response, responselen);
}

static void onUnsolicitedResponseShim(int unsolResponse, const void* data, size_t datalen) {
    if (!data) {
        ALOGV("%s: data is NULL", __func__);
        goto do_not_handle;
    }

    switch (unsolResponse) {
        case RIL_UNSOL_SIGNAL_STRENGTH:
            if (datalen != sizeof(RIL_SignalStrength_v10_vendor)) {
                ALOGE("%s: invalid response length", __func__);
                goto do_not_handle;
            }

            datalen = transformResponse(data);
            break;
    }

do_not_handle:
    ossRilEnv->OnUnsolicitedResponse(unsolResponse, data, datalen);
}

const RIL_RadioFunctions* RIL_Init(const struct RIL_Env* env, int argc, char** argv) {
    RIL_RadioFunctions const* (*qmiRilInit)(const struct RIL_Env* env, int argc, char** argv);
    static struct RIL_Env shimmedRilEnv;
    void* qmiRil;

    /*
     * Save the RilEnv passed from rild.
     */
    ossRilEnv = env;

    /*
     * Copy the RilEnv and shim the OnRequestComplete function.
     */
    shimmedRilEnv = *env;
    shimmedRilEnv.OnRequestComplete = onRequestCompleteShim;
    shimmedRilEnv.OnUnsolicitedResponse = onUnsolicitedResponseShim;

    /*
     * Open the qmi RIL.
     */
    qmiRil = dlopen(RIL_LIB_NAME, RTLD_LOCAL);
    if (!qmiRil) {
        ALOGE("%s: failed to load %s: %s\n", __func__, RIL_LIB_NAME, dlerror());
        return NULL;
    }

    /*
     * Get a reference to the qmi RIL_Init.
     */
    qmiRilInit = dlsym(qmiRil, "RIL_Init");
    if (!qmiRilInit) {
        ALOGE("%s: failed to find RIL_Init\n", __func__);
        goto fail_after_dlopen;
    }

    /*
     * Init the qmi RIL add pass it the shimmed RilEnv.
     */
    qmiRilFunctions = qmiRilInit(&shimmedRilEnv, argc, argv);
    if (!qmiRilFunctions) {
        ALOGE("%s: failed to get functions from RIL_Init\n", __func__);
        goto fail_after_dlopen;
    }

    return qmiRilFunctions;

fail_after_dlopen:
    dlclose(qmiRil);

    return NULL;
}
