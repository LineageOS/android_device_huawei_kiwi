#
# Copyright (C) 2014 The CyanogenMod Project
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
#

# Inherit from those products. Most specific first.
ifneq ($(TOMATO_32_BIT),true)
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
endif

TARGET_LOCALES := en_US en_IN en_GB hi_IN mr_IN ml_IN ta_IN kn_IN te_IN

$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit from tomato device
$(call inherit-product, device/yu/tomato/device.mk)

# Device identifier. This must come after all inclusions
PRODUCT_DEVICE := tomato
PRODUCT_NAME := full_tomato
PRODUCT_BRAND := YU
PRODUCT_MODEL := AO5510
PRODUCT_MANUFACTURER := YU
