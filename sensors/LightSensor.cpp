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

#include "sensors.h"
#include "LightSensor.h"

#define LIGHT_SYS_PATH      "/sys/devices/soc.0/78b6000.i2c/i2c-0/0-001e/"
#define LIGHT_SENSOR_NAME   "PA12200001_light"
#define LIGHT_ENABLE        "enable_als_sensor"
#define LIGHT_DATA          "als"
#define LIGHT_POLL_DELAY    "als_poll_delay"
#define EVENT_TYPE_LIGHT    EV_MSC
/*****************************************************************************/
LightSensor::LightSensor()
: SensorBase(NULL, LIGHT_SENSOR_NAME),
	  mInputReader(4),
	  mHasPendingEvent(false),
	  sensor_index(0)
{
	int i;

	mPendingEvent.version = sizeof(sensors_event_t);
	mPendingEvent.sensor = SENSORS_LIGHT_HANDLE;
	mPendingEvent.type = SENSOR_TYPE_LIGHT;
	memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));

	if (data_fd > 0) {
		strcpy(input_sysfs_path, LIGHT_SYS_PATH);
		input_sysfs_path_len = strlen(input_sysfs_path);
		ALOGI("The light sensor path is %s",input_sysfs_path);
		enable(0, 1);
	}
}

LightSensor::LightSensor(struct SensorContext *context)
	: SensorBase(NULL, LIGHT_SENSOR_NAME, context),
	  mInputReader(4),
	  mHasPendingEvent(false),
	  sensor_index(0)
{
	mPendingEvent.version = sizeof(sensors_event_t);
	mPendingEvent.sensor = context->sensor->handle;
	mPendingEvent.type = SENSOR_TYPE_LIGHT;
	memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));

	strlcpy(input_sysfs_path, LIGHT_SYS_PATH, sizeof(input_sysfs_path));
	input_sysfs_path_len = strlen(input_sysfs_path);

	mUseAbsTimeStamp = false;
	context->data_fd = data_fd;
}

LightSensor::~LightSensor() {
	if (mEnabled) {
		enable(0, 0);
	}
}

int LightSensor::setDelay(int32_t, int64_t ns)
{
	int fd;
	int delay_ms = ns / 1000000;
	strlcpy(&input_sysfs_path[input_sysfs_path_len],
			LIGHT_POLL_DELAY, SYSFS_MAXLEN);
	fd = open(input_sysfs_path, O_RDWR);
	if (fd >= 0) {
		char buf[80];
		snprintf(buf, sizeof(buf), "%d", delay_ms);
		write(fd, buf, strlen(buf)+1);
		close(fd);
		return 0;
	}
	return -1;
}

int LightSensor::enable(int32_t, int en)
{
	int flags = en ? 1 : 0;
	if (flags != mEnabled) {
		int fd;
		if (sensor_index >= 0) {
			strlcpy(&input_sysfs_path[input_sysfs_path_len],
				LIGHT_ENABLE, sizeof(input_sysfs_path) - input_sysfs_path_len);
		}
		else {
			ALOGE("invalid sensor index:%d\n", sensor_index);
			return -1;
		}

		fd = open(input_sysfs_path, O_RDWR);
		if (fd >= 0) {
			char buf[2];
			int err;
			buf[1] = 0;
			if (flags) {
				buf[0] = '1';
			} else {
				buf[0] = '0';
			}
			err = write(fd, buf, sizeof(buf));
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

bool LightSensor::hasPendingEvents() const {
	return mHasPendingEvent || mHasPendingMetadata;
}

int LightSensor::readEvents(sensors_event_t* data, int count)
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
		if (type == EVENT_TYPE_LIGHT) {
			if (event->code == MSC_SCAN) {
				mPendingEvent.light = event->value;
			}
		} else if (type == EV_SYN) {
			switch ( event->code ){
				case SYN_TIME_SEC:
					{
						mUseAbsTimeStamp = true;
						report_time = event->value*1000000000LL;
					}
				break;
				case SYN_TIME_NSEC:
					{
						mUseAbsTimeStamp = true;
						mPendingEvent.timestamp = report_time+event->value;
					}
				break;
				case SYN_REPORT:
					{
						if(mUseAbsTimeStamp != true) {
							mPendingEvent.timestamp = timevalToNano(event->time);
						}
						if (mEnabled) {
							*data++ = mPendingEvent;
							count--;
							numEventReceived++;
						}
					}
				break;
			}

		} else {
			ALOGE("LightSensor: unknown event (type=%d, code=%d)",
					type, event->code);
		}
		mInputReader.next();
	}

	return numEventReceived;
}

