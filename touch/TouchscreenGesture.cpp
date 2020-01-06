/*
 * Copyright (C) 2019-2020 The LineageOS Project
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

#define LOG_TAG "TouchscreenGestureService"

#include "TouchscreenGesture.h"
#include <fstream>
#include <map>
#include <type_traits>
#include <vector>

namespace vendor {
namespace lineage {
namespace touch {
namespace V1_0 {
namespace implementation {

const std::string kGesturePath = "/sys/touch_screen/easy_wakeup_gesture";

const std::map<int32_t, TouchscreenGesture::GestureInfo> TouchscreenGesture::kGestureInfoMap = {
    // clang-format off
    {0, {66, "Letter C"}},
    {1, {67, "Letter e"}},
    {2, {68, "Letter M"}},
    {3, {87, "Letter W"}},
    // clang-format on
};

const std::vector<uint16_t> kGestureMasks = {
    0x080,  // C gesture mask
    0x100,  // e gesture mask
    0x200,  // M gesture mask
    0x400,  // W gesture mask
};

Return<void> TouchscreenGesture::getSupportedGestures(getSupportedGestures_cb resultCb) {
    std::vector<Gesture> gestures;

    for (const auto& entry : kGestureInfoMap) {
        gestures.push_back({entry.first, entry.second.name, entry.second.keycode});
    }
    resultCb(gestures);

    return Void();
}

Return<bool> TouchscreenGesture::setGestureEnabled(
    const ::vendor::lineage::touch::V1_0::Gesture& gesture, bool enabled) {
    uint16_t gestureMode;
    uint16_t mask = kGestureMasks[gesture.id];
    std::string line;
    std::stringstream ss;
    std::fstream file(kGesturePath);

    file >> line;
    ss << std::hex << line.substr(2);
    ss >> gestureMode;

    if (enabled) {
        gestureMode |= mask;
    } else {
        gestureMode &= ~mask;
    }

    file << gestureMode;

    return !file.fail();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace lineage
}  // namespace vendor
