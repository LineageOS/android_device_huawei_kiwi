/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_PROXIMITY_SENSOR_H
#define ANDROID_PROXIMITY_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "SensorBase.h"
#include "InputEventReader.h"
#include "NativeSensorManager.h"

#define ALSPROX_IOCTL_MAGIC          (0xCF)
#define ALSPROX_IOCTL_PROX_ON        _IOW(ALSPROX_IOCTL_MAGIC, 3, unsigned long)
#define ALSPROX_IOCTL_PROX_OFF       _IOW(ALSPROX_IOCTL_MAGIC, 4, unsigned long)
#define ALSPROX_IOCTL_PROX_OFFSET    _IOW(ALSPROX_IOCTL_MAGIC, 5, unsigned long)
#define ALSPROX_IOCTL_PROX_CALIBRATE _IOW(ALSPROX_IOCTL_MAGIC, 6, unsigned long)
#define ALSPROX_IOCTL_PHONE_STATE    _IOW(ALSPROX_IOCTL_MAGIC, 7, unsigned long)

/*****************************************************************************/

struct input_event;

class ProximitySensor : public SensorBase {
    InputEventCircularReader mInputReader;
    sensors_event_t mPendingEvent;
    bool mHasPendingEvent;
    int sensor_index;

    int setInitialState();
    float indexToValue(size_t index) const;

public:
	ProximitySensor();
	ProximitySensor(char *name);
	ProximitySensor(struct SensorContext *context);
    virtual ~ProximitySensor();
    virtual int readEvents(sensors_event_t* data, int count);
    virtual bool hasPendingEvents() const;
    virtual int enable(int32_t handle, int enabled);
};

/*****************************************************************************/

#endif  // ANDROID_PROXIMITY_SENSOR_H
