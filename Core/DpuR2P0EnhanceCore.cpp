/*
 * Copyright (C) 2010 The Android Open Source Project
 * Copyright (C) 2012-2017, The Linux Foundation. All rights reserved.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
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

#include "DpuEnhanceCore.h"

struct slp_cfg slp_bypass = {
    .brightness = 16,
    .conversion_matrix = 0,
    .brightness_step = 1,
    .second_bright_factor = 16,
    .first_percent_th = 50,
    .first_max_bright_th = 48
};

int DpuR2P0EnhanceCore::slpSetBrightness(int ambient) {
    int brightness;
    int ret = 0;
    static uint8_t mode, index;

    if (abc->mode == ABC_MODE_DISABLE)
        return -1;

    brightness = ambientToBrightness(ambient);
    if (brightness < 0) {
        ALOGE("%s: wrong brightness ambient:%d\n", __func__, ambient);
        return -EINVAL;
    }

    if (mAbcEnable &&
        (mode == (abc->mode - 1)) && (index == abc->index)) {
        if (mBrightness == brightness) {
            abc->ambient = ambient;
            ALOGD("%s: ambient = %d\n", __func__, ambient);
            return 0;
        } else if ((mBrightness > brightness) &&
            ((abc->ambient - ambient) < SLP_AMBIENT_REDUCTION)) {
            ALOGD("%s: ambient = %d\n", __func__, ambient);
            return 0;
        }
    }

    ALOGD("%s: ambient = %d, brightness = %d\n", __func__,
                    ambient, brightness);

    abc->ambient = ambient;
    mode = abc->mode - 1;
    index = abc->index;
    abc->slp[mode][index].brightness = brightness;
    if (brightness > SLP_BRIGHTNESS_THRESHOLD) {
        /*enable slp before epf is enabled*/
        ret = slpRegWrite(&abc->slp[mode][index]);
        if (ret < 0) {
            ALOGE("%s: write slp register fail\n", __func__);
            return ret;
        }
    } else if (mBrightness > SLP_BRIGHTNESS_THRESHOLD) {
        ret = slpRegWrite(&slp_bypass);
        if (ret < 0) {
            ALOGE("%s: set slp bypass mode fail\n", __func__);
            return ret;
        }
    }

    mAbcEnable = 1;
    mBrightness = brightness;

    return 0;
}

int DpuR2P0EnhanceCore::abcSetMode(int mode) {
    int ret = 0;

    ALOGD("%s: mode = %d\n", __func__, mode);

    abc->mode = mode & 0x3;

    if (mode == ABC_MODE_DISABLE) {
        if (mBrightness > SLP_BRIGHTNESS_THRESHOLD) {
            ret = slpRegWrite(&slp_bypass);
            if (ret < 0) {
                ALOGE("%s: set slp bypass mode fail\n", __func__);
                return ret;
            }
        }
        mAbcEnable = 0;
        mBrightness = 0;
    } else {
            if (mode & ABC_MODE_GAME)
                abc->index = 0;
            else if (mode & ABC_MODE_CAMERA)
                abc->index = 1;
            else if (mode & ABC_MODE_VIDEO) {
                    if (mode & ABC_MODE_FULL_FRAME)
                        abc->index = 2;
                    else
                        abc->index = 3;
            } else if (mode & ABC_MODE_IMAGE)
                abc->index = 4;
            else if (mode & ABC_MODE_UI)
                abc->index = 5;

            ALOGD("%s: abc mode = %d index= %d\n", __func__,
                    abc->mode, abc->index);
    }

    if (mAbcEnable)
        slpSetBrightness(abc->ambient);

    return ret;
}
