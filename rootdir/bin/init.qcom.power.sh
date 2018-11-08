#!/vendor/bin/sh
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017 The LineageOS Project
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

# Set devfreq parameters for MSM8939
echo "cpufreq" > /sys/class/devfreq/mincpubw/governor
echo "bw_hwmon" > /sys/class/devfreq/cpubw/governor
echo "20" > /sys/class/devfreq/cpubw/bw_hwmon/io_percent
echo "40" > /sys/class/devfreq/gpubw/bw_hwmon/io_percent
