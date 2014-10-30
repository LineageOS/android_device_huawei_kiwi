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

#ifndef ANDROID_SENSORS_H
#define ANDROID_SENSORS_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <linux/input.h>

#include <hardware/hardware.h>
#include <hardware/sensors.h>

__BEGIN_DECLS

/*****************************************************************************/

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define SENSORS_ACCELERATION_HANDLE		0
#define SENSORS_MAGNETIC_FIELD_HANDLE		1
#define SENSORS_ORIENTATION_HANDLE		2
#define SENSORS_LIGHT_HANDLE			3
#define SENSORS_PROXIMITY_HANDLE		4
#define SENSORS_GYROSCOPE_HANDLE		5
#define SENSORS_PRESSURE_HANDLE			6

#define MAX_SENSORS		(20)
#define SYSFS_MAXLEN		(20)
#define SYSFS_CLASS		"/sys/class/input"
#define SYSFS_NAME		"name"
#define SYSFS_VENDOR		"vendor"
#define SYSFS_VERSION		"version"
#define SYSFS_HANDLE		"handle"
#define SYSFS_TYPE		"type"
#define SYSFS_MAXRANGE		"max_range"
#define SYSFS_RESOLUTION	"resolution"
#define SYSFS_POWER		"sensor_power"
#define SYSFS_MINDELAY		"min_delay"
#define SYSFS_ENABLE		"enable"
#define SYSFS_POLL_DELAY	"poll_delay"

#define COMPASS_VENDOR_AKM		"AKM"
#define COMPASS_VENDOR_ALPS		"Alps"
#define COMPASS_VENDOR_YAMAHA		"Yamaha"
#define COMPASS_VENDOR_MEMSIC		"Memsic"
#define COMPASS_VENDOR_ST		"STMicro"
#define COMPASS_VENDOR_BOSCH		"Bosch"
#define COMPASS_VENDOR_KIONIX		"Kionix"
#define COMPASS_VENDOR_INVENSENSE	"Invensense"


/*****************************************************************************/

__END_DECLS

#endif  // ANDROID_SENSORS_H
