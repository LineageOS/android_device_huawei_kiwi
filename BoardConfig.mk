#
# Copyright (C) 2015 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

-include device/cyanogen/msm8939-common/BoardConfigCommon.mk

LOCAL_PATH := device/yu/tomato

TARGET_SPECIFIC_HEADER_PATH := $(LOCAL_PATH)/include

# Assertions
TARGET_BOARD_INFO_FILE ?= device/yu/tomato/board-info.txt

# Kernel
TARGET_KERNEL_CONFIG := cyanogenmod_tomato-64_defconfig

# Bluetooth
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(LOCAL_PATH)/bluetooth

# Camera
BOARD_CAMERA_SENSORS := imx135_cp8675 imx214_cp8675 ov5648_cp8675

# CMHW
BOARD_HARDWARE_CLASS += $(LOCAL_PATH)/cmhw/src

# Lights
TARGET_PROVIDES_LIBLIGHT := true

# Partitions
BOARD_FLASH_BLOCK_SIZE := 131072
BOARD_BOOTIMAGE_PARTITION_SIZE := 20971520
BOARD_CACHEIMAGE_PARTITION_SIZE := 134217728
BOARD_PERSISTIMAGE_PARTITION_SIZE := 10485760
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 20971520
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 1258291200
BOARD_USERDATAIMAGE_PARTITION_SIZE := 13576175616 # 13576192000 - 16384

# Recovery
TARGET_RECOVERY_PIXEL_FORMAT := ABGR_8888

# SELinux
BOARD_SEPOLICY_DIRS += \
    device/yu/tomato/sepolicy

BOARD_SEPOLICY_UNION += \
    file.te \
    file_contexts \
    system_app.te \
    system_server.te \
    system.te \
    wcnss_service.te

# Video
TARGET_HAVE_SIGNED_VENUS_FW := true

# inherit from the proprietary version
-include vendor/yu/tomato/BoardConfigVendor.mk
