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

DpuLiteR2P0EnhanceCore::DpuLiteR2P0EnhanceCore()
            : mBld(NULL), mAbc(NULL), mCms(NULL) {
    if (bld) {
        free(bld);
        bld = (struct bld_context *)malloc(sizeof(struct dpu_lite_r2p0_bld_context));
        if (bld) {
            memset(bld, 0, sizeof(struct dpu_lite_r2p0_bld_context));
            mBld = (struct dpu_lite_r2p0_bld_context *)bld;
        }
    }

    if (abc) {
        free(abc);
        abc = (struct abc_context *)malloc(sizeof(struct dpu_lite_r2p0_abc_context));
        if (abc) {
            memset(abc, 0, sizeof(struct dpu_lite_r2p0_abc_context));
            mAbc = (struct dpu_lite_r2p0_abc_context *)abc;
        }
    }

    if (cms) {
        free(cms);
        cms = (struct cms_context *)malloc(sizeof(struct dpu_lite_r2p0_cms_context));
        if (cms) {
            memset(cms, 0, sizeof(struct dpu_lite_r2p0_cms_context));
            mCms = (struct dpu_lite_r2p0_cms_context *)cms;
        }
    }
}

DpuLiteR2P0EnhanceCore::~DpuLiteR2P0EnhanceCore() {
    if (bld) {
        free(bld);
        bld = NULL;
    }
    mBld = NULL;

    if (abc) {
        free(abc);
        abc = NULL;
    }
    mAbc = NULL;

    if (cms) {
        free(cms);
        cms = NULL;
    }
    mCms = NULL;
}

int DpuLiteR2P0EnhanceCore::slpLtmRegWrite(struct dpu_lite_r2p0_slp_ltm_cfg *slpLtmPtr)
{
    int fd;
    int ret;

    fd = open(FILE_LTM, O_RDWR);
    if (fd >= 0) {
        ret = write(fd, slpLtmPtr, sizeof(struct dpu_lite_r2p0_slp_ltm_cfg));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_LTM);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuLiteR2P0EnhanceCore::slpRegWrite(struct dpu_lite_r2p0_slp_ltm_cfg *slpLtmPtr)
{
    int fd;
    int ret;

    fd = open(FILE_SLP, O_RDWR);
    if (fd >= 0) {
        ret = write(fd, slpLtmPtr, sizeof(struct dpu_lite_r2p0_slp_ltm_cfg));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_SLP);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuLiteR2P0EnhanceCore::slpSetBrightness(int ambient) {
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
            ret = slpRegWrite(&mCms->slp_ltm[cms->mode - 1]);
            if (ret) {
                ALOGE("%s: write cms slp register fail\n", __func__);
                return ret;
            }

            ret = enhanceDisable(ENHANCE_LTM);
        }
    }

    mAbcEnable = 1;
    mBrightness = brightness;

    return 0;
}

int DpuLiteR2P0EnhanceCore::abcSetMode(int mode) {
    int ret = 0;

    ALOGD("%s: mode = %d\n", __func__, mode);

    abc->mode = mode & 0x3;

    if (mode == ABC_MODE_DISABLE) {
        if (mBrightness > SLP_BRIGHTNESS_THRESHOLD) {
            if (cms->mode == CMS_MODE_DISABLE)
                ret = enhanceDisable(ENHANCE_SLP | ENHANCE_LTM);
            else if (cms->mode == CMS_MODE_AUTO) {
                ret = slpRegWrite(&mCms->slp_ltm[cms->mode - 1]);
                if (ret) {
                    ALOGE("%s: write cms slp and ltm register fail\n", __func__);
                    return ret;
                }

                ret = enhanceDisable(ENHANCE_LTM);
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
        ret = slpSetBrightness(abc->ambient);

    return ret;
}

/* use formulation to calculate color matrix coefficients */
void DpuLiteR2P0EnhanceCore::levelToCoefficient(short *coef00,
                    short *coef11, short *coef22) {
    short value;
    uint8_t level;

    level = BLD_MODE_MAX - mBld->mode;

    value = (mBld->cm.coef00 *
        ((mBld->rgb.rmin << 6) + level * (mBld->rgb.rmax - mBld->rgb.rmin))) >> 16;
    *coef00 = mBld->cm.coef00 - value;

    value = (mBld->cm.coef11 *
        ((mBld->rgb.gmin << 6) + level * (mBld->rgb.gmax - mBld->rgb.gmin))) >> 16;
    *coef11 = mBld->cm.coef11 - value;

    value = (mBld->cm.coef22 *
        ((mBld->rgb.bmin << 6) + level * (mBld->rgb.bmax - mBld->rgb.bmin))) >> 16;
    *coef22 = mBld->cm.coef22 - value;
}

int DpuLiteR2P0EnhanceCore::cmsSetMode(int mode) {
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
    }

    if ((cms->mode != CMS_MODE_AUTO) ||
        (mBrightness <= SLP_BRIGHTNESS_THRESHOLD)) {
        ret = slpRegWrite(&mCms->slp_ltm[mode - 1]);
        if (ret) {
            ALOGE("%s: write slp register fail\n", __func__);
            return ret;
        }
        ret = enhanceDisable(ENHANCE_LTM);
    }

    /* If cms mode is standard, set gamma mode to standard */
    if ((gamma->num_modes == 2) && (cms->mode == CMS_MODE_STANDARD))
        gamma_mode = GAMMA_MODE_STANDARD;
    else
        gamma_mode = GAMMA_MODE_DEFAULT;

    ret = gammaSetMode(gamma_mode);

    return ret;
}

int DpuLiteR2P0EnhanceCore::bldSetMode(int mode) {
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

    levelToCoefficient(&cm.coef00, &cm.coef11, &cm.coef22);

    ret = cmRegWrite(&cm);

    if (!ret)
        ret = gammaSetMode(GAMMA_MODE_DEFAULT);

    return ret;
}

int DpuLiteR2P0EnhanceCore::gammaSetMode(int mode) {
    int ret;

    if (!gamma->num_modes) {
        ALOGD("gamma is not supported!");
        return 0;
    }

    if (mode && (gamma->mode == mode)) {
        ALOGV("gamma mode is identical, no need to set.");
        return 0;
    }

    ALOGD("%s: enable = %d\n", __func__, mode);

    gamma->mode = mode;
    if (mode) {
        ret = gammaRegWrite(&gamma->lut[mode - 1]);
    } else {
        ret = enhanceDisable(ENHANCE_GAMMA);
    }

    return ret;
}

int DpuLiteR2P0EnhanceCore::slpRegRead(void *buf) {
    int fd;
    int ret;

    fd = open(FILE_LTM, O_RDWR);
    if (fd >= 0) {
        ret = read(fd, buf, sizeof(struct dpu_lite_r2p0_slp_ltm_cfg));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_LTM);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuLiteR2P0EnhanceCore::slpGetValue(void) {
    int ret;
    struct dpu_lite_r2p0_slp_ltm_cfg cfg = {};

    pthread_mutex_lock(&g_lock);

    ALOGD("%s: start get value\n", __func__);

    ret = slpRegRead(&cfg);
    if (ret) {
        printf("get sunlight protector registers failed\n");
        pthread_mutex_unlock(&g_lock);
        return -EINVAL;
    }

    printf("brightness=%u\n", cfg.brightness);
    printf("conversion_matrix=%u\n", cfg.conversion_matrix);
    printf("brightness_step=%u\n", cfg.brightness_step);
    printf("second_bright_factor=%u\n", cfg.second_bright_factor);
    printf("first_percent_th=%u\n", cfg.first_percent_th);
    printf("first_max_bright_th=%u\n", cfg.first_max_bright_th);
    printf("low_clip=%u\n", (cfg.low_clip & 0x1ff));
    printf("high_clip=%u\n", (cfg.high_clip & 0x1ff));
    printf("step_clip=%u\n", (cfg.step_clip & 0xff));

    pthread_mutex_unlock(&g_lock);

    return ret;
}
