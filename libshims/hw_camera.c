/*
 * Copyright (C) 2016 The CyanogenMod Project
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

#define LOG_TAG "hw_camera_shim"
#include <cutils/log.h>

#include <dlfcn.h>
#include <stdio.h>

static int (*__srget_real)(FILE *fp) = NULL;

int __srget(FILE *fp)
{
	if (!__srget_real) {
		__srget_real = dlsym(RTLD_NEXT, "__srget");
		if (!__srget_real) {
			ALOGE("Real __srget not found, returning EOF");
			return EOF;
		}
	}

	return __srget_real(fp);
}

