/*--------------------------------------------------------------------------
Copyright (c) 2014, The Linux Foundation. All rights reserved.

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

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--------------------------------------------------------------------------*/

#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <utils/Log.h>
#include <CalibrationModule.h>
#include "CalibrationManager.h"

CalibrationManager* CalibrationManager::self = NULL;

int CalibrationManager::check_algo(const sensor_cal_algo_t *list)
{
	if (list->tag != SENSOR_CAL_ALGO_TAG)
		return -1;
	if ((list->type < SENSOR_TYPE_ACCELEROMETER) ||
			(list->type > SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR))
		return -1;
	if (list->compatible == NULL)
		return -1;
	if (list->module == NULL)
		return -1;
	if (list->methods == NULL)
		return -1;
	return 0;
}

/* Return 0 on success */
void CalibrationManager::loadCalLibs()
{
	int fd;
	size_t len;
	char buf[MAX_CAL_CFG_LEN];
	char* cal_libs[MAX_CAL_LIBS];
	struct sensor_cal_module_t* modules[MAX_CAL_LIBS];
	int i = 0;
	int count;
	int tmp;

	algo_count = 0;
	algo_list = NULL;

	cal_libs[0] = strdup(DEFAULT_CAL_LIB);
	cal_libs[1] = NULL;
	count = 1;

	fd = open(CAL_LIB_CFG_PATH, O_RDONLY);
	if (fd < 0) {
		ALOGE("Open %s failed.(%s)\nDrop to default calibration library.",
				CAL_LIB_CFG_PATH, strerror(errno));
	} else {
		len = read(fd, buf, MAX_CAL_CFG_LEN);
		if (len > 0) {
			char *save_ptr, *str, *token;

			buf[len] = '\0';
			for(str = buf; ;str = NULL) {
				token = strtok_r(str, "\n", &save_ptr);
				if (token == NULL)
					break;
				cal_libs[i++] = strdup(token);
			}
			cal_libs[i] = NULL;
			count = i;
		}
	}

	/* Load the libraries */
	for (i = 0; i < count; i++) {
		void* dso;

		ALOGI("Found calibration library:%s\n", cal_libs[i]);

		dso = dlopen(cal_libs[i], RTLD_NOW);
		if (dso == NULL) {
			char const *err_str = dlerror();
			ALOGE("load module %s failed(%s)", cal_libs[i], err_str?err_str:"unknown");
			modules[i] = NULL;
			free(cal_libs[i]);
			continue;
		}

		free(cal_libs[i]);

		modules[i] = (sensor_cal_module_t*)dlsym(dso, SENSOR_CAL_MODULE_INFO_AS_STR);
		if (modules[i] == NULL) {
			ALOGE("Can't find symbol %s\n", SENSOR_CAL_MODULE_INFO_AS_STR);
			continue;
		}

		modules[i]->dso = dso;

		if (modules[i]->methods->init(modules[i])) {
			ALOGE("init %s failed\n", modules[i]->id);
			modules[i] = NULL;
			continue;
		}
		algo_count += modules[i]->number;
	}

	if (algo_count != 0) {
		tmp = 0;

		algo_list = new const sensor_cal_algo_t *[algo_count];
		/* Get the algo list */
		for (i = 0; i < count; i++) {
			const sensor_cal_algo_t *list;
			/* Success */
			if ((modules[i] != NULL) && (modules[i]->methods != NULL) &&
					(modules[i]->methods->get_algo_list != NULL) &&
					(modules[i]->methods->get_algo_list(&list) == 0)) {
				for (uint32_t j = 0; j < modules[i]->number; j++)
					algo_list[tmp + j] = &list[j];

				tmp += modules[i]->number;
			}
		}
	}

	dump();
}

CalibrationManager::CalibrationManager()
	:algo_list(NULL), algo_count(0)
{
	loadCalLibs();
}

CalibrationManager::~CalibrationManager()
{
	if (algo_list != NULL) {
		for (uint32_t i = 0; i < algo_count; i++) {
			if ((algo_list[i]->module) && (algo_list[i]->module->dso))
				dlclose(algo_list[i]->module->dso);
		}
		delete[] algo_list;
	}
	self = NULL;
}

CalibrationManager* CalibrationManager::defaultCalibrationManager()
{
	if (self == NULL) {
		self = new CalibrationManager;
	}

	return self;
}

const char* CalibrationManager::type_to_name(int type)
{
	switch (type) {
		case SENSOR_TYPE_ACCELEROMETER:
			return ACCELEROMETER_NAME;
		case SENSOR_TYPE_GEOMAGNETIC_FIELD:
			return COMPASS_NAME;
		case SENSOR_TYPE_ORIENTATION:
			return ORIENTATION_NAME;
		case SENSOR_TYPE_GYROSCOPE:
			return GYROSCOPE_NAME;
		case SENSOR_TYPE_LIGHT:
			return LIGHT_NAME;
		case SENSOR_TYPE_PRESSURE:
			return PRESSURE_NAME;
		case SENSOR_TYPE_TEMPERATURE:
			return TEMPERATURE_NAME;
		case SENSOR_TYPE_PROXIMITY:
			return PROXIMITY_NAME;
		case SENSOR_TYPE_GRAVITY:
			return GRAVITY_NAME;
		case SENSOR_TYPE_LINEAR_ACCELERATION:
			return LINEAR_ACCELERATION_NAME;
		case SENSOR_TYPE_ROTATION_VECTOR:
			return ROTATION_VECTOR_NAME;
		case SENSOR_TYPE_RELATIVE_HUMIDITY:
			return RELATIVE_HUMIDITY_NAME;
		case SENSOR_TYPE_AMBIENT_TEMPERATURE:
			return AMBIENT_TEMPERATURE_NAME;
		case SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED:
			return MAGNETIC_FIELD_UNCALIBRATED_NAME;
		case SENSOR_TYPE_GAME_ROTATION_VECTOR:
			return GAME_ROTATION_VECTOR_NAME;
		case SENSOR_TYPE_GYROSCOPE_UNCALIBRATED:
			return GYROSCOPE_UNCALIBRATED_NAME;
		case SENSOR_TYPE_SIGNIFICANT_MOTION:
			return SIGNIFICANT_MOTION_NAME;
		case SENSOR_TYPE_STEP_DETECTOR:
			return STEP_DETECTOR_NAME;
		case SENSOR_TYPE_STEP_COUNTER:
			return STEP_COUNTER_NAME;
		case SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR:
			return GEOMAGNETIC_ROTATION_VECTOR_NAME;
		default:
			return "";
	}
}

const sensor_cal_algo_t* CalibrationManager::getCalAlgo(const sensor_t *s/* = NULL*/)
{
	uint32_t i = 0;
	int j = 0;
	const sensor_cal_algo_t **list = algo_list;
	const sensor_cal_algo_t *tmp = NULL;

	if (s == NULL) {
		ALOGW("No available algo found!");
		return NULL;
	}

	for (i = 0; i < algo_count; i++) {
		if ((list[i]->type != s->type) || check_algo(list[i]))
			continue;
		j = 0;
		while (list[i]->compatible[j] != NULL) {
			if (strcmp(list[i]->compatible[j], s->name) == 0)
				break;
			if (strcmp(list[i]->compatible[j], type_to_name(s->type)) == 0)
				tmp = list[i];
			j++;
		}

		/* Exactly compatible */
		if (list[i]->compatible[j] != NULL)
			break;
	}

	if (i != algo_count) {
		ALOGI("found exactly compatible algo for type %d", s->type);
		return list[i];
	}

	if (tmp != NULL)
		ALOGI("found compatible algo for type %d", s->type);

	return tmp;
}

void CalibrationManager::dump()
{
	int j;
	uint32_t i;

	ALOGI("algo_count:%d\n", algo_count);
	for (i = 0; i < algo_count; i++) {
		ALOGI("tag:%d\tversion:%d\ttype:%d\n", algo_list[i]->tag, algo_list[i]->version, algo_list[i]->type);
		j = 0;
		while (algo_list[i]->compatible[j] != NULL) {
			ALOGI("compatible:%s\n", algo_list[i]->compatible[j++]);
		}
	}
}
