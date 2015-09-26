#
# Copyright (C) 2016  Rudolf Tammekivi <rtammekivi@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see [http://www.gnu.org/licenses/].
#

ifeq ($(TARGET_PROVIDES_WCNSS_QMI),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libhuawei_secure \
    liboeminfo_oem_api

LOCAL_SRC_FILES := \
    wcnss_qmi.c

LOCAL_MODULE := libwcnss_qmi
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -Wall

include $(BUILD_SHARED_LIBRARY)

endif
