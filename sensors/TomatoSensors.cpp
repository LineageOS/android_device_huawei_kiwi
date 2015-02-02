/*--------------------------------------------------------------------------
Copyright (c) 2014, The CyanogenMod Project

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
--------------------------------------------------------------------------*/

#include "TomatoSensors.h"

#define NUM_STATIC_SENSORS 4

struct sensor_t TomatoSensors::static_sensors [NUM_STATIC_SENSORS] = {
    [0] = {
        .name = "MPU6880 3-axis Accelerometer",
        .vendor = "Invensense",
        .version = 1,
        .handle = SENSORS_ACCELERATION_HANDLE,
        .type = SENSOR_TYPE_ACCELEROMETER,
        .maxRange = (2.0f * 9.81f),
        .resolution = 1.0,
        .power = 0.23f, /* mA */
        .minDelay = 20000,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
#if defined(SENSORS_DEVICE_API_VERSION_1_3)
        .stringType = NULL,
        .requiredPermission = NULL,
        .maxDelay = 5000000,
        .flags = SENSOR_FLAG_CONTINUOUS_MODE,
#endif
        .reserved = {},
    },
    [1] = {
        .name = "PA12200001 Light",
        .vendor = "TXC",
        .version = 1,
        .handle = SENSORS_LIGHT_HANDLE,
        .type = SENSOR_TYPE_LIGHT,
        .maxRange = 100000.0f,
        .resolution = 1.0f,
        .power = 0.1f,
        .minDelay = 20000,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
#if defined(SENSORS_DEVICE_API_VERSION_1_3)
        .stringType = NULL,
        .requiredPermission = NULL,
        .maxDelay = 0,
        .flags = SENSOR_FLAG_ON_CHANGE_MODE,
#endif
        .reserved = {},
    },

    [2] = {
        .name = "PA12200001 Proximity",
        .vendor = "TXC",
        .version = 1,
        .handle = SENSORS_PROXIMITY_HANDLE,
        .type = SENSOR_TYPE_PROXIMITY,
        .maxRange = 5.0f,
        .resolution = 1.0f,
        .power = 0.1f, /* mA */
        .minDelay = 0,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
#if defined(SENSORS_DEVICE_API_VERSION_1_3)
        .stringType = NULL,
        .requiredPermission = NULL,
        .maxDelay = 0,
        .flags = SENSOR_FLAG_WAKE_UP|SENSOR_FLAG_ON_CHANGE_MODE,
#endif
        .reserved = {},
    },

    [3] = {
        .name = "MPU6880 Gyroscope",
        .vendor = "Invensense",
        .version = 1,
        .handle = SENSORS_GYROSCOPE_HANDLE,
        .type = SENSOR_TYPE_GYROSCOPE,
        .maxRange = 2000.0f,
        .resolution = 0.006f,
        .power = 6.1f, /* mA */
        .minDelay = 1200,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
#if defined(SENSORS_DEVICE_API_VERSION_1_3)
        .stringType = NULL,
        .requiredPermission = NULL,
        .maxDelay = 5000000,
        .flags = SENSOR_FLAG_CONTINUOUS_MODE,
#endif
        .reserved = {},
    },
};


int TomatoSensors::getStaticSensors(struct SensorContext *context, int num) {

    struct SensorContext *list;

    for (int i = 0; i < NUM_STATIC_SENSORS; i++) {
        list = &context[num + i];
        *(list->sensor) = TomatoSensors::static_sensors[i];
        list->sensor->handle = SENSORS_HANDLE(num + i);
    }

    return NUM_STATIC_SENSORS;
}
