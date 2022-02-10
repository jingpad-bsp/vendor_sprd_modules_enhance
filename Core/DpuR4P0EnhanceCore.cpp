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
#include <string.h>

DpuR4P0EnhanceCore::DpuR4P0EnhanceCore(){
    mAbc = NULL;
    mCms = NULL;

    if (abc) {
        free(abc);
        abc = (struct abc_context *)malloc(sizeof(struct dpu_r4p0_abc_context));
        if (abc) {
            memset(abc, 0, sizeof(struct dpu_r4p0_abc_context));
            mAbc = (struct dpu_r4p0_abc_context *)abc;
        }
    }

    if (cms) {
        free(cms);
        cms = (struct cms_context *)malloc(sizeof(struct dpu_r4p0_cms_context));
        if (cms) {
            memset(cms, 0, sizeof(struct dpu_r4p0_cms_context));
            mCms = (struct dpu_r4p0_cms_context *)cms;
        }
    }
}

DpuR4P0EnhanceCore::~DpuR4P0EnhanceCore() {
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

int DpuR4P0EnhanceCore::slpLtmRegWrite(struct dpu_r4p0_slp_ltm_cfg *slpLtmPtr) {
    int fd;
    int ret;

    fd = open(FILE_LTM, O_RDWR);
    if (fd >= 0) {
        ret = write(fd, slpLtmPtr, sizeof(struct dpu_r4p0_slp_ltm_cfg));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_LTM);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuR4P0EnhanceCore::slpRegRead(void *buf) {
    int fd;
    int ret;

    fd = open(FILE_LTM, O_RDWR);
    if (fd >= 0) {
        ret = read(fd, buf, sizeof(struct dpu_r4p0_slp_ltm_cfg));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_LTM);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

static void cmsInitSlp(struct dpu_r4p0_slp_ltm_cfg *slpLtmPtr) {
    int i;

    slpLtmPtr->hist_exb_no = 2;
    slpLtmPtr->hist_exb_percent = 25;
    for (i = 0; i < 9; i++)
        slpLtmPtr->hist9_index[i] = 15 * i;
    slpLtmPtr->glb_x[0] = 20;
    slpLtmPtr->glb_x[1] = 60;
    slpLtmPtr->glb_x[2] = 120;
    slpLtmPtr->glb_s[0] = 240;
    slpLtmPtr->glb_s[1] = 64;
    slpLtmPtr->glb_s[2] = 0;
    slpLtmPtr->fast_ambient_th = 3;
    slpLtmPtr->scene_change_percent_th = 80;
    slpLtmPtr->local_weight = 0;
    slpLtmPtr->brightness_step = 64;
    slpLtmPtr->first_max_bright_th = 80;
    for (i = 0; i < 5; i++)
        slpLtmPtr->first_max_bright_th_step[i] = 5 * (i + 1);
    slpLtmPtr->mask_height = 25;
    for (i = 0; i < 4; i++)
        slpLtmPtr->first_pth_index[i] = i;
    slpLtmPtr->fst_pth = 10;
}

int DpuR4P0EnhanceCore::lut3dRegWrite(struct threed_lut *lut3dPtr) {
    int fd;
    int ret;

    fd = open(FILE_LUT3D, O_RDWR);
    if (fd >= 0) {
        ret = write(fd, lut3dPtr, sizeof(struct threed_lut));
        close(fd);
    } else {
        ALOGE("%s: open %s fail\n", __func__, FILE_LUT3D);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuR4P0EnhanceCore::cmsSetMode(int mode) {
    int ret = 0, i;
    int gamma_mode = GAMMA_MODE_DEFAULT;
    int disable;

    if (mCms->slp_ltm[0].hist_exb_no == 0) {
        for (i = 0; i < 3; i++)
            cmsInitSlp(&mCms->slp_ltm[i]);
    }

    cms->mode = mode;

    if (mode == CMS_MODE_DISABLE) {
        /* if the BLD is active, don't disable cm */
        if (bld->mode == BLD_MODE_DISABLE)
            disable = ENHANCE_HSV | ENHANCE_CM | ENHANCE_LUT3D;
        else
            disable = ENHANCE_HSV | ENHANCE_LUT3D;

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

    ALOGD("%s: mode:%d mBrightness:%d \n", __func__, cms->mode, mBrightness);

    if ((cms->mode != CMS_MODE_AUTO) ||
        (mBrightness <= SLP_BRIGHTNESS_THRESHOLD)) {
        ret = slpLtmRegWrite(&mCms->slp_ltm[mode - 1]);
        if (ret) {
            ALOGE("%s: write slp and ltm register fail\n", __func__);
            return ret;
        }
    }

    /* If cms mode is standard, set gamma mode to standard */
    if (cms->mode == CMS_MODE_STANDARD)
        gamma_mode = GAMMA_MODE_STANDARD;

    ret = gammaSetMode(gamma_mode);
    if (ret) {
        ALOGE("%s: write gamma register fail\n", __func__);
        return ret;
    }

    /* enable 3dlut when gamma is enabled */
    if (gamma->mode && mCms->lut3d_support)
        ret = lut3dRegWrite(&mCms->lut3d[mode - 1]);

    return ret;
}

int DpuR4P0EnhanceCore::slpSetBrightness(int ambient) {
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

    ALOGD("%s: ambient = %d, brightness = %d\n", __func__, ambient, brightness);

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
                ALOGE("%s: disable slp and ltm fail\n", __func__);
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

int DpuR4P0EnhanceCore::abcSetMode(int mode) {
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

int DpuR4P0EnhanceCore::gammaSetMode(int mode) {
    int ret;

    if ((!gamma->num_modes) || (mode > 2)) {
        ALOGD("gamma is not supported!");
        return 0;
    }

    if (mode && (gamma->mode == mode)) {
        ALOGV("gamma mode is identical, no need to set.");
        return 0;
    }

    ALOGD("%s: enable = %d\n", __func__, mode);

    gamma->mode = mode;
    if (mode)
        ret = gammaRegWrite(&gamma->lut[mode - 1]);
    else
        ret = enhanceDisable(ENHANCE_GAMMA | ENHANCE_DITHER);

    return ret;
}

int DpuR4P0EnhanceCore::slpGetValue(void) {
    int ret;
    struct dpu_r4p0_slp_ltm_cfg cfg = {};

    pthread_mutex_lock(&g_lock);

    ALOGD("%s: start get value\n", __func__);

    ret = slpRegRead(&cfg);
    if (ret) {
        printf("get sunlight protector registers failed\n");
        pthread_mutex_unlock(&g_lock);
        return -EINVAL;
    }

    printf("brightness:%u\n", cfg.brightness);
    printf("brightness_step:%u\n", cfg.brightness_step);
    printf("first_max_bright_th:%u\n", cfg.first_max_bright_th);
    printf("low_clip:%u\n", (cfg.limit_lclip & 0x1ff));
    printf("high_clip:%u\n", (cfg.limit_hclip & 0x1ff));
    printf("step_clip:%u\n", (cfg.limit_clip_step & 0xff));
    printf("first_percent_th:%d  scene_change_percent_th:%d  fast_ambient_th:%d\n",
           cfg.fst_pth, cfg.scene_change_percent_th, cfg.fast_ambient_th);
    printf("hist_exb_percent:%d  hist_exb_no:%d\n", cfg.hist_exb_percent,
           cfg.hist_exb_no);
    printf("mask_height:%d  local_weight:%d\n", cfg.mask_height,
           cfg.local_weight);
    printf("first_max_bright_th:%d  first_max_bright_th_step0:%d  "
           "first_max_bright_th_step1:%d\n",
           cfg.first_max_bright_th, cfg.first_max_bright_th_step[0],
           cfg.first_max_bright_th_step[1]);
    printf("first_max_bright_th_step2:%d  first_max_bright_th_step3:%d  "
           "first_max_bright_th_step4:%d\n",
           cfg.first_max_bright_th_step[2], cfg.first_max_bright_th_step[3],
           cfg.first_max_bright_th_step[4]);
    printf("first_pth_index[0]:%d  first_pth_index[1]:%d  "
           "first_pth_index[2]:%d  first_pth_index[3]:%d\n",
           cfg.first_pth_index[0], cfg.first_pth_index[1],
           cfg.first_pth_index[2], cfg.first_pth_index[3]);
    printf("hist9_index0:%d  hist9_index1:%d  hist9_index2:%d\n",
           cfg.hist9_index[0], cfg.hist9_index[1], cfg.hist9_index[2]);
    printf("hist9_index3:%d  hist9_index4:%d  hist9_index5:%d\n",
           cfg.hist9_index[3], cfg.hist9_index[4], cfg.hist9_index[5]);
    printf("hist9_index6:%d  hist9_index7:%d  hist9_index8:%d\n",
           cfg.hist9_index[6], cfg.hist9_index[7], cfg.hist9_index[8]);
    printf("glb_x1:%d  glb_x2:%d  glb_x3:%d\n", cfg.glb_x[0], cfg.glb_x[1],
           cfg.glb_x[2]);
    printf("glb_s1:%d  glb_s2:%d  glb_s3:%d\n", cfg.glb_s[0], cfg.glb_s[1],
           cfg.glb_s[2]);

    pthread_mutex_unlock(&g_lock);

    return ret;
}

int DpuR4P0EnhanceCore::lut3dRegRead(void *buf) {
    int fd;
    int ret;

    fd = open(FILE_LUT3D, O_RDWR);
    if (fd >= 0) {
        ret = read(fd, buf, sizeof(struct threed_lut));
        close(fd);
    } else {
        ALOGE("%s: open %s fail\n", __func__, FILE_LUT3D);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuR4P0EnhanceCore::gammaGetValue(void) {
    int ret;
    int i;
    struct gamma_lut gam = {};
    struct threed_lut lut = {};

    pthread_mutex_lock(&g_lock);

    ALOGD("%s: start get value\n", __func__);

    ret = gammaRegRead(&gam);
    if (ret) {
        printf("get gamma registers failed\n");
        pthread_mutex_unlock(&g_lock);
        return -EINVAL;
    }

    printf("gamma:\n");

    for (i = 0; i < 256; i++)
        printf("0x%02x: r=%u, g=%u, b=%u\n", i,
                gam.r[i], gam.g[i], gam.b[i]);

    ret = lut3dRegRead(&lut);
    if (ret) {
        printf("get lut3d registers fail\n");
        pthread_mutex_unlock(&g_lock);
        return -EINVAL;
    }

    printf("3dlut:\n");

    for (i = 0; i < 729; i++)
        printf("0x%02x: r=%u, g=%u, b=%u\n", i,
            (lut.value[i] >> 20) & 0x3FF,
            (lut.value[i] >> 10) & 0x3FF,
             lut.value[i] & 0x3FF);

    pthread_mutex_unlock(&g_lock);

    return ret;
}
