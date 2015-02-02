/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Not a Contribution.
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

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>

#include <cutils/log.h>

#include "ProximitySensor.h"
#include "sensors.h"

#define PROXIMITY_SYS_PATH      "/sys/devices/soc.0/78b6000.i2c/i2c-0/0-001e/"
#define PROXIMITY_SEN_NAME      "PA12200001_proximity"
#define PROXIMITY_ENABLE        "enable_ps_sensor"
#define ALSPROX_DEV_NAME     "/dev/yl_alsprox_sensor"
#define EVENT_TYPE_PROXIMITY    EV_MSC

/*****************************************************************************/
ProximitySensor::ProximitySensor()
    : SensorBase(NULL, PROXIMITY_SEN_NAME),
      mInputReader(4),
      mHasPendingEvent(false),
      sensor_index(0)
{
	int i;

	mPendingEvent.version = sizeof(sensors_event_t);
	mPendingEvent.sensor = SENSORS_PROXIMITY_HANDLE;
	mPendingEvent.type = SENSOR_TYPE_PROXIMITY;
	memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));

	if (data_fd) {
		enable(0, 1);
	}
}

ProximitySensor::ProximitySensor(struct SensorContext *context)
	: SensorBase(NULL, PROXIMITY_SEN_NAME, context),
	  mInputReader(4),
	  mHasPendingEvent(false),
	  sensor_index(0)
{
	mPendingEvent.version = sizeof(sensors_event_t);
	mPendingEvent.sensor = context->sensor->handle;
	mPendingEvent.type = SENSOR_TYPE_PROXIMITY;
	memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));

    if (data_fd > 0) {
        context->data_fd = data_fd;
    }
}

ProximitySensor::~ProximitySensor() {
    if (mEnabled) {
        enable(0, 0);
    }
}

int ProximitySensor::enable(int32_t, int en) {
    int flags = en ? 1 : 0;
    if (flags != mEnabled) {
        int fd;
        if (sensor_index < 0) {
            ALOGE("invalid sensor index:%d\n", sensor_index);
            return -1;
        }

        fd = open(ALSPROX_DEV_NAME, O_RDWR);
        if (fd >= 0) {
            if (flags) {
                ioctl(fd, ALSPROX_IOCTL_PROX_ON, NULL);
            } else {
                ioctl(fd, ALSPROX_IOCTL_PROX_OFF, NULL);
            }
            close(fd);
            mEnabled = flags;
            return 0;
        } else {
            ALOGE("open %s failed.(%s)\n", input_sysfs_path, strerror(errno));
            return -1;
        }
	} else if (flags) {
		mHasPendingEvent = true;
	}
    return 0;
}

bool ProximitySensor::hasPendingEvents() const {
    return mHasPendingEvent || mHasPendingMetadata;
}

int ProximitySensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
        return -EINVAL;

    if (mHasPendingEvent) {
        mHasPendingEvent = false;
        mPendingEvent.timestamp = getTimestamp();
        *data = mPendingEvent;
        return mEnabled ? 1 : 0;
    }

	if (mHasPendingMetadata) {
		mHasPendingMetadata--;
		meta_data.timestamp = getTimestamp();
		*data = meta_data;
		return mEnabled ? 1 : 0;
	}

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
        return n;

    int numEventReceived = 0;
    input_event const* event;

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        if (type == EVENT_TYPE_PROXIMITY) {
            if (event->code == MSC_SCAN) {
                if (event->value >= 5)
                    mPendingEvent.distance = 5;
                else
                    mPendingEvent.distance = 0;
            }
        } else if (type == EV_SYN) {
                switch ( event->code ) {
                        case SYN_TIME_SEC:
                                mUseAbsTimeStamp = true;
                                report_time = event->value * 1000000000LL;
                                break;
                        case SYN_TIME_NSEC:
                                mUseAbsTimeStamp = true;
                                mPendingEvent.timestamp = report_time + event->value;
                                break;
                        case SYN_REPORT:
                                if(mUseAbsTimeStamp != true) {
                                        mPendingEvent.timestamp = timevalToNano(event->time);
                                }
                                if (mEnabled) {
                                        *data++ = mPendingEvent;
                                        count--;
                                        numEventReceived++;
                                }
                                break;
                }

        } else {
            ALOGE("ProximitySensor: unknown event (type=%d, code=%d)",
                    type, event->code);
        }
        mInputReader.next();
    }

    return numEventReceived;
}
