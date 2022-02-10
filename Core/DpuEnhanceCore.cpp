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
#include "EnhanceModule.h"
#include "DpuEnhanceCore.h"

DpuEnhanceCore::DpuEnhanceCore() : mAbcEnable(0), mCount(0), mBrightness(0) {
    if (abc == NULL) {
        abc = (abc_context *)malloc(sizeof(struct abc_context));
        if (abc)
            memset(abc, 0, sizeof(struct abc_context));
    }

    if (bld == NULL) {
        bld = (bld_context *)malloc(sizeof(struct bld_context));
        if (bld)
            memset(bld, 0, sizeof(struct bld_context));
    }

    if (cms == NULL) {
        cms = (cms_context *)malloc(sizeof(struct cms_context));
        if (cms)
            memset(cms, 0, sizeof(struct cms_context));
    }

    if (gamma == NULL) {
        gamma = (gamma_context *)malloc(sizeof(struct gamma_context));
        if (gamma)
            memset(gamma, 0, sizeof(struct gamma_context));
    }

    if (flash == NULL) {
        flash = (flash_context *)malloc(sizeof(struct flash_context));
        if (flash)
            memset(flash, 0, sizeof(struct flash_context));
    }
}

DpuEnhanceCore::~DpuEnhanceCore() {
    if (abc) {
        free(abc);
        abc = NULL;
    }

    if (bld) {
        free(bld);
        bld = NULL;
    }

    if (cms) {
        free(cms);
        cms = NULL;
    }

    if (gamma) {
        free(gamma);
        gamma = NULL;
    }

    if (flash) {
        free(flash);
        flash = NULL;
    }
}

int DpuEnhanceCore::enhanceDisable(uint32_t module) {
    int ret;
    int fd;

    fd = open(FILE_DISABLE, O_WRONLY);
    if (fd >= 0) {
        ret = write(fd, &module, sizeof(module));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_DISABLE);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

uint32_t DpuEnhanceCore::enhanceStatus(void) {
    int ret, fd;
    char buf[12] = {};

    fd = open(FILE_STATUS, O_RDONLY);
    if (fd < 0) {
        ALOGE("%s: open %s failed\n", __func__, FILE_STATUS);
        return 0;
    }

    ret = read(fd, buf, sizeof(buf));
    close(fd);

    if (ret < 0) {
        ALOGE("%s: read %s failed\n", __func__, FILE_STATUS);
        return 0;
    }

    return strtoul(buf, NULL, 16);
}

int DpuEnhanceCore::ambientToBrightness(int ambient) {
    int i, count;
    int ret = -EINVAL;
    uint8_t mode, index;

    mode = abc->mode - 1;
    index = abc->index;

    count = sizeof(struct amb_mapping) / sizeof(struct amb_item);

    for (i = 0; i < count; i++) {
        if (abc->map[mode][index].item[i].ambient == 0)
            continue;

        if (ambient <= abc->map[mode][index].item[i].ambient)
            return abc->map[mode][index].item[i].brightness;
        else
            ret = abc->map[mode][index].item[i].brightness;
    }

    return ret;
}

int DpuEnhanceCore::slpRegWrite(struct slp_cfg *slpPtr) {
    int fd;
    int ret;

    fd = open(FILE_SLP, O_RDWR);
    if (fd >= 0) {
        ret = write(fd, slpPtr, sizeof(struct slp_cfg));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_SLP);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuEnhanceCore::epfRegWrite(struct epf_cfg *epfPtr) {
    int fd;
    int ret;

    fd = open(FILE_EPF, O_RDWR);
    if (fd >= 0) {
        ret = write(fd, epfPtr, sizeof(struct epf_cfg));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_EPF);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuEnhanceCore::slpSetBrightness(int ambient) {
    int brightness, epf_pending = 0;
    int ret = 0;
    static uint8_t mode, index;

    if (abc->mode == ABC_MODE_DISABLE)
        return -1;

    brightness = ambientToBrightness(ambient);
    if (brightness < 0)
        return -EINVAL;

    ALOGD("%s: ambient = %d, brightness = %d\n", __func__,
                    ambient, brightness);

    abc->ambient = ambient;
    if (mAbcEnable &&
        (mode == (abc->mode - 1)) && (index == abc->index)) {
        if (mBrightness == brightness) {
            ALOGV("%s:brightness is identical\n", __func__);
            return 0;
        }
    } else
        epf_pending = 1;

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

        if (epf_pending || mBrightness <= SLP_BRIGHTNESS_THRESHOLD) {
            ret = epfRegWrite(&abc->epf_slp[mode][index]);
            epf_pending = 0;
        }

    } else if (mBrightness > SLP_BRIGHTNESS_THRESHOLD) {
        ret = enhanceDisable(ENHANCE_SLP | ENHANCE_EPF);
        if (ret < 0) {
            ALOGE("%s: disable slp and epf fail\n", __func__);
            return ret;
        }
        epf_pending = 1;
    }

    if (epf_pending)
        ret = epfRegWrite(&abc->epf_sr[mode][index]);

    mAbcEnable = 1;
    mBrightness = brightness;

    return (ret < 0) ? -errno : 0;
}

int DpuEnhanceCore::abcSetMode(int mode) {
    int ret = 0;

    ALOGD("%s: mode = %d\n", __func__, mode);

    abc->mode = mode & 0x3;

    if (mode == ABC_MODE_DISABLE) {
        ret = enhanceDisable(ENHANCE_SLP | ENHANCE_EPF);
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

int DpuEnhanceCore::rgbToIndex(int rgb) {
    int i, count;

    count = sizeof(cms->map) / sizeof(cms->map[0]);

    for (i = 0; i < count; i++) {
        if (rgb <= cms->map[i].rgb)
            return cms->map[i].index;
    }

    ALOGE("%s: error: rgb=%d is out of range\n", __func__, rgb);
    return -EINVAL;
}

int DpuEnhanceCore::cmRegWrite(struct cm_cfg *cmPtr) {
    int fd, ret;

    fd = open(FILE_CM, O_RDWR);
    if (fd >= 0) {
        ret = write(fd, cmPtr, sizeof(struct cm_cfg));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_CM);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuEnhanceCore::hsvRegWrite(struct hsv_lut *hsvPtr) {
    int fd;
    int ret = 0;

    fd = open(FILE_HSV, O_RDWR);
    if (fd >= 0) {
        ret = write(fd, hsvPtr, sizeof(struct hsv_lut));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_HSV);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuEnhanceCore::cmsSetMode(int mode) {
    int ret = 0;

    ALOGD("%s: mode = %d\n", __func__, mode);

    cms->mode = mode;

    if (mode == CMS_MODE_DISABLE) {
        /* if the BLD is active, don't disable cm & hsv */
        if (bld->mode == BLD_MODE_DISABLE)
            ret = enhanceDisable(ENHANCE_HSV | ENHANCE_CM);

        return ret;
    }

    ret = hsvRegWrite(&cms->hsvcm[mode - 1].hsv);
    if (ret < 0) {
        ALOGE("%s: write hsv register fail\n", __func__);
        return ret;
    }

    /*If the mode is auto, enable cm after temperature is set*/
    if (cms->mode != CMS_MODE_AUTO)
        ret = cmRegWrite(&cms->hsvcm[mode - 1].cm);

    return ret;
}

int DpuEnhanceCore::cmsSetTemperature(int temp) {
    int ret;
    int index;
    struct cm_cfg *cm;

    cms->temp = temp;
    if (temp & 0xFF000000) {
        ALOGD("%s: temperature = %u\n", __func__, temp & 0xFF);

        index = temp & 0xFF;
        cm = &cms->cm_manual[index];
    } else {
        ALOGD("%s: rgb sensor = %u\n", __func__, temp & 0xFFFFFF);

        index = rgbToIndex(temp);
        if (index < 0)
            return -EINVAL;

        cm = &cms->cm_auto[index];
    }

    ret = cmRegWrite(cm);

    return ret;
}

static int temperatureToIndex(int temp) {
    int i;

    for (i = 0; i < 16; i++) {
        if (flash->temp[i] == temp)
            break;
    }

    if (i == 16) {
        ALOGE("%s: find temperature %d failed\n", __func__, temp);
        return -1;
    }

    return i;
}

int DpuEnhanceCore::flashSetTemperature(int temp) {
    int ret = 0;

    ALOGD("%s: temperature = %d\n", __func__, temp);

    if (temp) {

        int index = temperatureToIndex(temp);
        if (index < 0) {
            return -1;
        }

        if (!flash->has_backup) {
            uint32_t status = enhanceStatus();
            if (status & ENHANCE_CM) {
                cmRegRead(&flash->cm_backup);
                flash->has_backup = 1;
            }
        }

        ret = cmRegWrite(&flash->cm[index]);

        flash->is_enabled = 1;

    } else if (flash->is_enabled) {

        if (flash->has_backup)
            ret = cmRegWrite(&flash->cm_backup);
        else
            ret = enhanceDisable(ENHANCE_CM);

        flash->is_enabled = 0;
        flash->has_backup = 0;
    }

    return ret;
}

int DpuEnhanceCore::bldSetMode(int mode) {
    int ret = 0;

    ALOGD("%s: mode = %d\n", __func__, mode);

    bld->mode = mode;

    if (mode == BLD_MODE_DISABLE) {
        /* if the CMS is active, don't disable cm & hsv */
        if (cms->mode == CMS_MODE_DISABLE)
            ret = enhanceDisable(ENHANCE_HSV | ENHANCE_CM);

        return ret;
    }

    ret = hsvRegWrite(&bld->hsvcm[mode - 1].hsv);
    if (ret < 0) {
        ALOGE("%s: write hsv register fail\n", __func__);
        return ret;
    }

    ret = cmRegWrite(&bld->hsvcm[mode - 1].cm);

    return ret;
}

int DpuEnhanceCore::slpRegRead(void *buf) {
    int fd;
    int ret;

    fd = open(FILE_SLP, O_RDWR);
    if (fd >= 0) {
        ret = read(fd, buf, sizeof(struct slp_cfg));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_SLP);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuEnhanceCore::slpGetValue(void) {
    int ret;
    struct slp_cfg cfg = {};

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

    pthread_mutex_unlock(&g_lock);

    return ret;
}

int DpuEnhanceCore::cmRegRead(void *buf) {
    int fd;
    int ret;

    fd = open(FILE_CM, O_RDWR);
    if (fd >= 0) {
        ret = read(fd, buf, sizeof(struct cm_cfg));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_CM);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuEnhanceCore::cmGetValue(void) {
    int ret;
    struct cm_cfg cfg = {};

    pthread_mutex_lock(&g_lock);

    ALOGD("%s: start get value\n", __func__);

    ret = cmRegRead(&cfg);
    if (ret) {
        printf("get color matrix registers failed\n");
        pthread_mutex_unlock(&g_lock);
        return -EINVAL;
    }

    printf("coef00=%d\n", cfg.coef00);
    printf("coef01=%d\n", cfg.coef01);
    printf("coef02=%d\n", cfg.coef02);
    printf("coef03=%d\n", cfg.coef03);
    printf("coef10=%d\n", cfg.coef10);
    printf("coef11=%d\n", cfg.coef11);
    printf("coef12=%d\n", cfg.coef12);
    printf("coef13=%d\n", cfg.coef13);
    printf("coef20=%d\n", cfg.coef20);
    printf("coef21=%d\n", cfg.coef21);
    printf("coef22=%d\n", cfg.coef22);
    printf("coef23=%d\n", cfg.coef23);

    pthread_mutex_unlock(&g_lock);

    return ret;
}

int DpuEnhanceCore::hsvRegRead(void *buf) {
    int fd;
    int ret;

    fd = open(FILE_HSV, O_RDWR);
    if (fd >= 0) {
        ret = read(fd, buf, sizeof(struct hsv_lut));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_HSV);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuEnhanceCore::hsvGetValue(void) {
    int ret;
    int i;
    struct hsv_lut lut = {};

    pthread_mutex_lock(&g_lock);

    ALOGD("%s: start get value\n", __func__);

    ret = hsvRegRead(&lut);
    if (ret) {
        printf("get hsv registers failed\n");
        pthread_mutex_unlock(&g_lock);
        return -EINVAL;
    }

    for (i = 0; i < 360; i++)
        printf("0x%02x: hue=%u, sat=%u\n", i,
            lut.table[i].hue, lut.table[i].sat);

    pthread_mutex_unlock(&g_lock);

    return ret;
}

int DpuEnhanceCore::epfRegRead(void *buf) {
    int fd;
    int ret;

    fd = open(FILE_EPF, O_RDWR);
    if (fd >= 0) {
        ret = read(fd, buf, sizeof(struct epf_cfg));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_EPF);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuEnhanceCore::epfGetValue(void) {
    int ret;
    struct epf_cfg cfg = {};

    pthread_mutex_lock(&g_lock);

    ALOGD("%s: start get value\n", __func__);

    ret = epfRegRead(&cfg);
    if (ret) {
        printf("get epf registers failed\n");
        pthread_mutex_unlock(&g_lock);
        return -EINVAL;
    }

    printf("epsilon0=%d\n", cfg.epsilon0);
    printf("epsilon1=%d\n", cfg.epsilon1);
    printf("gain0=%d\n", cfg.gain0);
    printf("gain1=%d\n", cfg.gain1);
    printf("gain2=%d\n", cfg.gain2);
    printf("gain3=%d\n", cfg.gain3);
    printf("gain4=%d\n", cfg.gain4);
    printf("gain5=%d\n", cfg.gain5);
    printf("gain6=%d\n", cfg.gain6);
    printf("gain7=%d\n", cfg.gain7);
    printf("max_diff=%d\n", cfg.max_diff);
    printf("min_diff=%d\n", cfg.min_diff);

    pthread_mutex_unlock(&g_lock);

    return ret;
}

int DpuEnhanceCore::gammaRegRead(void *buf) {
    int fd;
    int ret;

    fd = open(FILE_GAMMA, O_RDWR);
    if (fd >= 0) {
        ret = read(fd, buf, sizeof(struct gamma_lut));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_GAMMA);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuEnhanceCore::gammaGetValue(void) {
    int ret;
    int i;
    struct gamma_lut gam = {};

    pthread_mutex_lock(&g_lock);

    ALOGD("%s: start get value\n", __func__);

    ret = gammaRegRead(&gam);
    if (ret) {
        printf("get gamma registers failed\n");
        pthread_mutex_unlock(&g_lock);
        return -EINVAL;
    }

    for (i = 0; i < 256; i++)
        printf("0x%02x: r=%u, g=%u, b=%u\n", i,
                gam.r[i], gam.g[i], gam.b[i]);

    pthread_mutex_unlock(&g_lock);

    return ret;
}

int DpuEnhanceCore::gammaRegWrite(struct gamma_lut *gammaPtr) {
    int fd;
    int ret;

    fd = open(FILE_GAMMA, O_RDWR);
    if (fd >= 0) {
        ret = write(fd, gammaPtr, sizeof(struct gamma_lut));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_GAMMA);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int DpuEnhanceCore::gammaSetMode(int mode) {
    int ret;

    ALOGD("%s: enable = %d\n", __func__, mode);

    if (!gamma->num_modes) {
        ALOGD("gamma is not supported!");
        return 0;
    }

    gamma->mode = mode;
    if (mode) {
        ret = gammaRegWrite(&gamma->lut[mode - 1]);
    } else {
        ret = enhanceDisable(ENHANCE_GAMMA);
    }

    return ret;
}

int DpuEnhanceCore::cabcSetMode(uint32_t mode) {
    ALOGD("%s: do not support CABC\n", __func__);
    return -1;
}

int DpuEnhanceCore::cabcGetValue() {
    ALOGD("%s: do not support CABC\n", __func__);
    return -1;
}

