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

#include "Cabc.h"
#include "DpuEnhanceCore.h"
#include <string.h>
#include <stdlib.h>

DpuR3P0EnhanceCore::DpuR3P0EnhanceCore() : lastBrightness(0) {
    mCabc = NULL;
    mCabc = (struct dpu_r3p0_cabc_context *)malloc(sizeof(struct dpu_r3p0_cabc_context));
    if (mCabc)
        memset(mCabc, 0, sizeof(struct dpu_r3p0_cabc_context));

#ifdef SPRD_CABC
    cabc_ctr = new CABC();
#endif
}

DpuR3P0EnhanceCore::~DpuR3P0EnhanceCore() {
    if (mCabc) {
        free(mCabc);
    }
    mCabc = NULL;

#ifdef SPRD_CABC
    if (cabc_ctr) {
        delete cabc_ctr;
        cabc_ctr = NULL;
    }
#endif
}

int DpuR3P0EnhanceCore::cabcEnable(uint32_t mode) {
    int ret;
    int fd;
    int cabc_switch, cabc_status;
    uint32_t cabc_mode = mCabc->user_mode & 0xFFFFFFFE;
    fd = open(FILE_CABC_ENABLE, O_RDWR);
    if (fd >= 0) {
        ret = read(fd, &cabc_status, sizeof(int));
        if (ret <= 0) {
            close(fd);
            ALOGE("%s: cabc read status failed\n", __func__);
            return ret;
        }

        if (((mode & CABC_ENABLE) && (cabc_status != 0)) ||
           (cabc_mode != (mCabc->kernel_mode & 0xFFFFFFFE))) {
            ret = cabcModeWrite(cabc_mode);
        #ifdef SPRD_CABC
            cabc_ctr->cabcMode(cabc_mode);
            ALOGD("%s: cabc_mode is %d\n", __func__, cabc_mode);
        #endif
            if (ret) {
                close(fd);
                ALOGE("%s: cabc set mode failed\n", __func__);
                return ret;
            }
        }

        cabc_switch = (mode & CABC_ENABLE) ? 0 : 1;
        ALOGD("%s: read mode[%d] set mode[%d]\n", __func__, cabc_status, cabc_switch);

        if (((cabc_status != 0) && (cabc_switch == 0)) ||
        ((cabc_status == 0) && (cabc_switch != 0))) {
            ret = write(fd, &cabc_switch, sizeof(int));
            if (ret <= 0) {
                close(fd);
                ALOGE("%s: write cabc_switch failed, errno is %d\n", __func__, errno);
                return ret;
            }
        } else
            ALOGD("%s: The current mode is the same as the parameter setting mode\n", __func__);

        mCabc->kernel_mode = cabc_mode | (mode & CABC_ENABLE);
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_CABC_ENABLE);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuR3P0EnhanceCore::cabcModeWrite(uint32_t mode) {
    int ret;
    int fd;

    fd = open(FILE_CABC_MODE, O_WRONLY);
    if (fd >= 0) {
        ret = write(fd, &mode, sizeof(uint32_t));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_CABC_MODE);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

bool DpuR3P0EnhanceCore::cabcIsEnable(int mode) {
    bool ret;

    if ((mode & CABC_MODE_CAMERA) || !(mode & CABC_ENABLE))
        ret = false;
    else
        ret = true;

    return ret;
}

int DpuR3P0EnhanceCore::cmsSetTemperature(int temp) {
    int ret;
    int index;
    struct cm_cfg *cm;
    uint32_t cabc_mode = CABC_DISABLE;

    cms->temp = temp;
    if (temp & 0xFF000000) {
        ALOGD("%s: temperature = %u\n", __func__, temp & 0xFF);
        if ((temp == 0xFF000000) && (mBrightness <= CABC_BRIGHTNESS_THRESHOLD))
            cabc_mode = CABC_ENABLE;

        index = temp & 0xFF;
        cm = &cms->cm_manual[index];
    } else {
        ALOGD("%s: rgb sensor = %u\n", __func__, temp & 0xFFFFFF);

        index = rgbToIndex(temp);
        if (index < 0)
            return -EINVAL;

        cm = &cms->cm_auto[index];
    }

    if ((mCabc->kernel_mode & CABC_ENABLE) != cabc_mode) {
        if (cabcIsEnable(mCabc->user_mode)) {
            ret = cabcEnable(cabc_mode);
            if (ret < 0) {
                ALOGE("%s: cabc set fail\n", __func__);
                return ret;
            }
        }
    }

    ret = cmRegWrite(cm);

    return ret;
}

int DpuR3P0EnhanceCore::cmsSetMode(int mode) {
    int ret = 0;
    int gamma_mode;
    int disable;

    ALOGD("%s: mode = %d\n", __func__, mode);

    cms->mode = mode;

    if (mode == CMS_MODE_DISABLE) {
        /* if the BLD is active, don't disable cm */
        if (bld->mode == BLD_MODE_DISABLE)
            disable = ENHANCE_HSV | ENHANCE_CM;
        else
            disable = ENHANCE_HSV;

        if (mBrightness <= SLP_BRIGHTNESS_THRESHOLD)
            disable |= (ENHANCE_SLP | ENHANCE_LTM);

        ret = enhanceDisable(disable);

        return ret;
    }

    ret = hsvRegWrite(&cms->hsvcm[mode - 1].hsv);
    if (ret < 0) {
        ALOGE("%s: write hsv register fail\n", __func__);
        return ret;
    }

    /*If the mode is auto, enable cm after temperature is set*/
    if (cms->mode != CMS_MODE_AUTO) {
        ret = cmRegWrite(&cms->hsvcm[mode - 1].cm);
        if (ret < 0) {
            ALOGE("%s: write cm register fail\n", __func__);
            return ret;
        }
        if (cabcIsEnable(mCabc->user_mode)) {
            ret = cabcEnable(CABC_ENABLE);
            if (ret < 0) {
                ALOGE("%s: cabc enable fail\n", __func__);
                return ret;
            }
        }
    }

    if ((cms->mode != CMS_MODE_AUTO) ||
        (mBrightness <= SLP_BRIGHTNESS_THRESHOLD)) {
        ret = slpLtmRegWrite(&mCms->slp_ltm[mode - 1]);
        if (ret) {
            ALOGE("%s: write slp and ltm register fail\n", __func__);
            return ret;
        }
    }

    /* If cms mode is standard, set gamma mode to standard */
    if ((gamma->num_modes == 2) && (cms->mode == CMS_MODE_STANDARD))
        gamma_mode = GAMMA_MODE_STANDARD;
    else
        gamma_mode = GAMMA_MODE_DEFAULT;

    ret = gammaSetMode(gamma_mode);

    return ret;
}
int DpuR3P0EnhanceCore::bldSetMode(int mode) {
    int ret = 0;
    struct cm_cfg cm = {};

    ALOGD("%s: mode = %d\n", __func__, mode);

    bld->mode = mode;

    if (mode == BLD_MODE_DISABLE) {
        /* if the CMS is active, don't disable cm */
        if (cms->mode == CMS_MODE_DISABLE)
            ret = enhanceDisable(ENHANCE_CM);

        return ret;
    }

    if (mCabc->kernel_mode & CABC_ENABLE) {
        if (cabcIsEnable(mCabc->user_mode)) {
            ret = cabcEnable(CABC_DISABLE);
            if (ret < 0) {
                ALOGE("%s: cabc disable fail\n", __func__);
                return ret;
            }
        }
    }

    levelToCoefficient(&cm.coef00, &cm.coef11, &cm.coef22);

    ret = cmRegWrite(&cm);

    if (!ret)
        ret = gammaSetMode(GAMMA_MODE_DEFAULT);

    return ret;
}

int DpuR3P0EnhanceCore::slpSetBrightness(int ambient) {
    int brightness;
    int ret = 0;
    static uint8_t mode, index;
    bool cabc_change = false;

    if (abc->mode == ABC_MODE_DISABLE)
        return -1;

    brightness = ambientToBrightness(ambient);
    if (brightness < 0) {
        ALOGE("%s: wrong brightness ambient:%d\n", __func__, ambient);
        return -EINVAL;
    }

    if ((cms->mode == CMS_MODE_AUTO) && (cms->temp == 0xFF000000) &&
       cabcIsEnable(mCabc->user_mode) && (lastBrightness != brightness) &&
        ((brightness > CABC_BRIGHTNESS_THRESHOLD) && (lastBrightness <= CABC_BRIGHTNESS_THRESHOLD) ||
        (brightness <= CABC_BRIGHTNESS_THRESHOLD) && (lastBrightness > CABC_BRIGHTNESS_THRESHOLD))) {
        if (brightness > CABC_BRIGHTNESS_THRESHOLD) {
            ret = cabcEnable(CABC_DISABLE);
            if (ret) {
                ALOGE("%s: cabc set mode failed\n", __func__);
                return ret;
            }
        } else {
            cabc_change = true;
        }
    }
    lastBrightness = brightness;

    if (cabc_change) {
        ret = cabcEnable(CABC_ENABLE);
        if (ret) {
            ALOGE("%s: cabc set mode failed\n", __func__);
            return ret;
        }
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
    mAbc->slp_ltm[mode][index].brightness = brightness;
    if (brightness > SLP_BRIGHTNESS_THRESHOLD) {
        ret = slpLtmRegWrite(&mAbc->slp_ltm[mode][index]);
        if (ret < 0) {
            ALOGE("%s: write slp and ltm register fail\n", __func__);
            return ret;
        }

    } else if (mBrightness > SLP_BRIGHTNESS_THRESHOLD) {
        if (cms->mode == CMS_MODE_DISABLE) {
            ret = enhanceDisable(ENHANCE_SLP | ENHANCE_LTM);
            if (ret < 0) {
                ALOGE("%s: disable slp, ltm fail\n", __func__);
                return ret;
            }
        } else if (cms->mode == CMS_MODE_AUTO) {
            ret = slpLtmRegWrite(&mCms->slp_ltm[cms->mode - 1]);
            if (ret) {
                ALOGE("%s: write cms slp and ltm register fail\n", __func__);
                return ret;
            }
        }
    }

    mAbcEnable = 1;
    mBrightness = brightness;
    return 0;
}

int DpuR3P0EnhanceCore::abcSetMode(int mode) {
    int ret = 0;

    ALOGD("%s: mode = %d\n", __func__, mode);

    abc->mode = mode & 0x3;

    if (mode == ABC_MODE_DISABLE) {
        if (mBrightness > SLP_BRIGHTNESS_THRESHOLD) {
            if (cms->mode == CMS_MODE_DISABLE)
                ret = enhanceDisable(ENHANCE_SLP | ENHANCE_LTM);
            else if (cms->mode == CMS_MODE_AUTO) {
                ret = slpLtmRegWrite(&mCms->slp_ltm[cms->mode - 1]);
                if (ret) {
                    ALOGE("%s: write cms slp and ltm register fail\n", __func__);
                    return ret;
                }
            }
        }
        if ((mBrightness > CABC_BRIGHTNESS_THRESHOLD) && (bld->mode == BLD_MODE_DISABLE))
            if (cabcIsEnable(mCabc->user_mode) && (cms->temp == 0xFF000000)) {
                ret = cabcEnable(CABC_ENABLE);
            }

        mAbcEnable = 0;
        mBrightness = 0;
        lastBrightness = 0;
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
        ret = slpSetBrightness(abc->ambient);

    return ret;
}

int DpuR3P0EnhanceCore::cabcSetMode(uint32_t mode) {
    int ret = 0;

    #ifdef SPRD_CABC
    /* Vblank signal trigger cabc function */
    static int cabc_flag = 0;
    if (mode == CABC_VSYNC) {
        cabc_ctr->cabcVsyncRun();
        return 0;
    }
    /* flip signal trigger cabc function */
    if (mode == CABC_FLIP) {
        cabc_ctr->cabcFlipRun();
        return 0;
    }
    if (mode & CABC_MODE_VIDEO)
        cabc_flag = 1;

    if ((mode & CABC_MODE_UI) && cabc_flag) {
        cabc_flag = 0;
        cabc_ctr->cabcSwitchMode();
    }
    #endif

    ALOGD("%s: cabc mode:0x%02x\n", __func__, mode);

    mCabc->user_mode = mode;

    if ((mCms->mode == CMS_MODE_DISABLE) || (mBld->mode != BLD_MODE_DISABLE))
        return 0;

    if ((mode & CABC_MODE_CAMERA) || (mode == CABC_DISABLE)) {
        ret = cabcEnable(CABC_DISABLE);
        return ret;
    }

    if ((mCms->mode != CMS_MODE_AUTO) ||
       ((mCms->mode == CMS_MODE_AUTO) && (mCms->temp == 0xFF000000) &&
       (mBrightness <= CABC_BRIGHTNESS_THRESHOLD)))
        ret = cabcEnable(CABC_ENABLE);

    return ret;
}

int DpuR3P0EnhanceCore::cabcGetValue(void) {
    int ret;
    int fd;
    int cabc_switch = 1;

    fd = open(FILE_CABC_ENABLE, O_RDWR);
    if (fd >= 0) {
        ret = read(fd, &cabc_switch, sizeof(int));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_CABC_ENABLE);
        ret = -1;
    }

    if (cabc_switch != 0)
        printf("cabc status: off\n");
    else
        printf("cabc status: on\n");

    return ret;
}
