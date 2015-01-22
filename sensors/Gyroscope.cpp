/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  http://www.apache.org/licenses/LICENSE-2.0
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

#include "GyroSensor.h"
#include "sensors.h"

#define GYRO_INPUT_DEV_NAME 	"mpu6880"
#define GYRO_SYSFS_PATH			"/sys/class/input/input1/"
#define GYRO_ENABLE				"gyro_enable"
#define GYRO_DELAY				"gyro_delay"

#define FETCH_FULL_EVENT_BEFORE_RETURN 	1
#define IGNORE_EVENT_TIME 				350000000

#define	EVENT_TYPE_GYRO_X	ABS_RX
#define	EVENT_TYPE_GYRO_Y	ABS_RY
#define	EVENT_TYPE_GYRO_Z	ABS_RZ

#define GYROSCOPE_CONVERT	(M_PI / (180 * 16.4))
#define CONVERT_GYRO_X		(-GYROSCOPE_CONVERT)
#define CONVERT_GYRO_Y		( GYROSCOPE_CONVERT)
#define CONVERT_GYRO_Z		(-GYROSCOPE_CONVERT)

#define SMOOTHING_FACTOR    0.2

/*****************************************************************************/

GyroSensor::GyroSensor()
	: SensorBase(NULL, GYRO_INPUT_DEV_NAME),
	  mInputReader(4),
	  mHasPendingEvent(false),
	  mEnabledTime(0)
{
	mPendingEvent.version = sizeof(sensors_event_t);
	mPendingEvent.sensor = SENSORS_GYROSCOPE_HANDLE;
	mPendingEvent.type = SENSOR_TYPE_GYROSCOPE;
	memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));

	if (data_fd) {
		strcpy(input_sysfs_path, GYRO_SYSFS_PATH);
		input_sysfs_path_len = strlen(input_sysfs_path);
		ALOGI("The gyroscope sensor path is %s",input_sysfs_path);
		enable(0, 1);
	}
}

GyroSensor::GyroSensor(struct SensorContext *context)
	: SensorBase(NULL, GYRO_INPUT_DEV_NAME, context),
	  mInputReader(4),
	  mHasPendingEvent(false),
	  mEnabledTime(0)
{
	mPendingEvent.version = sizeof(sensors_event_t);
	mPendingEvent.sensor = context->sensor->handle;
	mPendingEvent.type = SENSOR_TYPE_GYROSCOPE;
	memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));
	mPendingEvent.gyro.status = SENSOR_STATUS_ACCURACY_HIGH;

	strlcpy(input_sysfs_path, GYRO_SYSFS_PATH, sizeof(input_sysfs_path));
	input_sysfs_path_len = strlen(input_sysfs_path);
	context->data_fd = data_fd;
	ALOGI("The gyroscope sensor path is %s",input_sysfs_path);
	mUseAbsTimeStamp = false;
	enable(0, 1);
}

GyroSensor::~GyroSensor() {
	if (mEnabled) {
		enable(0, 0);
	}
}

int GyroSensor::setInitialState() {
	struct input_absinfo absinfo_x;
	struct input_absinfo absinfo_y;
	struct input_absinfo absinfo_z;
	float value;
	if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_GYRO_X), &absinfo_x) &&
		!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_GYRO_Y), &absinfo_y) &&
		!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_GYRO_Z), &absinfo_z)) {
		value = absinfo_x.value;
		mAvgX = value;
		mPendingEvent.data[0] = value * CONVERT_GYRO_X;
		value = absinfo_y.value;
		mAvgY = value;
		mPendingEvent.data[1] = value * CONVERT_GYRO_Y;
		value = absinfo_z.value;
		mAvgZ = value;
		mPendingEvent.data[2] = value * CONVERT_GYRO_Z;
		mHasPendingEvent = true;
	}
	return 0;
}

int GyroSensor::enable(int32_t, int en) {
	int flags = en ? 1 : 0;
	if (flags != mEnabled) {
		int fd;
		strlcpy(&input_sysfs_path[input_sysfs_path_len],
				GYRO_ENABLE, SYSFS_MAXLEN);
		fd = open(input_sysfs_path, O_RDWR);
		if (fd >= 0) {
			char buf[2];
			int err;
			buf[1] = 0;
			if (flags) {
				buf[0] = '1';
				mEnabledTime = getTimestamp() + IGNORE_EVENT_TIME;
			} else {
				buf[0] = '0';
			}
			err = write(fd, buf, sizeof(buf));
			close(fd);
			mEnabled = flags;
			setInitialState();
			return 0;
		}
		return -1;
	}
	return 0;
}

bool GyroSensor::hasPendingEvents() const {
	return mHasPendingEvent || mHasPendingMetadata;
}

int GyroSensor::setDelay(int32_t, int64_t delay_ns)
{
	int fd;
	int delay_ms = delay_ns / 1000000;
	strlcpy(&input_sysfs_path[input_sysfs_path_len],
			GYRO_DELAY, SYSFS_MAXLEN);
	fd = open(input_sysfs_path, O_RDWR);
	if (fd >= 0) {
		char buf[80];
		sprintf(buf, "%d", delay_ms);
		write(fd, buf, strlen(buf)+1);
		close(fd);
		return 0;
	}
	return -1;
}

int GyroSensor::readEvents(sensors_event_t* data, int count)
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

#if FETCH_FULL_EVENT_BEFORE_RETURN
again:
#endif
	while (count && mInputReader.readEvent(&event)) {
		int type = event->type;
		if (type == EV_ABS) {
			float value = event->value;
			if (event->code == EVENT_TYPE_GYRO_X) {
				mAvgX = (value * SMOOTHING_FACTOR) + (mAvgX * (1 - SMOOTHING_FACTOR));
				mPendingEvent.data[0] = mAvgX * CONVERT_GYRO_X;
			} else if (event->code == EVENT_TYPE_GYRO_Y) {
				mAvgY = (value * SMOOTHING_FACTOR) + (mAvgY * (1 - SMOOTHING_FACTOR));
				mPendingEvent.data[1] = mAvgY * CONVERT_GYRO_Y;
			} else if (event->code == EVENT_TYPE_GYRO_Z) {
				mAvgZ = (value * SMOOTHING_FACTOR) + (mAvgZ * (1 - SMOOTHING_FACTOR));
				mPendingEvent.data[2] = mAvgZ * CONVERT_GYRO_Z;
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
							if(mPendingEvent.timestamp >= mEnabledTime) {
								*data++ = mPendingEvent;
								numEventReceived++;
							}
							count--;
						}
					}
				break;
			}
		} else {
			ALOGE("GyroSensor: unknown event (type=%d, code=%d)",
					type, event->code);
		}
		mInputReader.next();
	}

#if FETCH_FULL_EVENT_BEFORE_RETURN
	/* if we didn't read a complete event, see if we can fill and
	   try again instead of returning with nothing and redoing poll. */
	if (numEventReceived == 0 && mEnabled == 1) {
		n = mInputReader.fill(data_fd);
		if (n)
			goto again;
	}
#endif

	return numEventReceived;
}

