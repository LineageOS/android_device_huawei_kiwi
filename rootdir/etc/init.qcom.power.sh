#!/system/bin/sh
#
# Copyright (C) 2016 The CyanogenMod Project
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

export PATH=/system/bin

if [ -f /sys/devices/soc0/soc_id ]; then
    soc_hwid=`cat /sys/devices/soc0/soc_id`
else
    soc_hwid=`cat /sys/devices/system/soc/soc0/id`
fi

# Set devfreq parameters for MSM8939
case "$soc_hwid" in
    233|239|240|241|242|243|263|268|269|270|271)
        for devfreq_gov in /sys/class/devfreq/qcom,mincpubw*/governor
        do
            echo "cpufreq" > $devfreq_gov
        done

        for devfreq_gov in /sys/class/devfreq/qcom,cpubw*/governor
        do
            echo "bw_hwmon" > $devfreq_gov
        done

        for cpu_io_percent in /sys/class/devfreq/qcom,cpubw*/bw_hwmon/io_percent
        do
            echo 20 > $cpu_io_percent
        done

        for gpu_bimc_io_percent in /sys/class/devfreq/qcom,gpubw*/bw_hwmon/io_percent
        do
            echo 40 > $gpu_bimc_io_percent
        done
        ;;
esac
