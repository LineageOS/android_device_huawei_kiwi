/*
 * Copyright (C) 2014, The CyanogenMod Project
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

#include <sys/types.h>

#define YL_PARAM_WLAN_MAC 0
#define YL_PARAM_BT_MAC 1
#define YL_PARAM_IMEI0 2
#define YL_PARAM_IMEI1 3

int yl_get_param(int param, void *buf, size_t len);
int yl_params_init(void);
