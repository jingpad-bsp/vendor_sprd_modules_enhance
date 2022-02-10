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

#include "cabc_definition.h"
#include "Cabc.h"

CABC::CABC () {
    cabc_step0 = 8;
    cabc_step1 = 72;
    cabc_step2 = 28;
    cabc_step3 = 0;
    cabc_scene_change_thr = 80;
    cabc_percent_thr_u = 1;
    cabc_percent_thr_v = 10;
    cabc_min_backlight = 408;
    frame_no = 0;
    init_flag = 1;
    cabc_no = 0;
    last_bl = 0;
    cabc_switch = 1;
    cabc_flag = 0;
}

CABC::~CABC() {}

void CABC::cabcMode(uint32_t cabc_mode) {

        if (cabc_mode & CABC_MODE_UI)
            cabc_para.video_mode = 0;
        else if (cabc_mode & CABC_MODE_FULL_FRAME)
            cabc_para.video_mode = 1;
        else if (cabc_mode & CABC_MODE_VIDEO)
            cabc_para.video_mode = 1;
        ALOGD("enhance CABC mode: 0x%x\n", cabc_mode);
}

static void cabcInit(int vactive, int hactive) {

    init_cabc(vactive, hactive);
}

int CABC::cabcHistRead(uint32_t hist[16]) {
    int ret;
    int fd;
    fd = open(FILE_CABC_HIST, O_RDONLY);
    if (fd >= 0) {
        ret =read(fd, hist, 16*sizeof(uint32_t));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_CABC_HIST);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int CABC::cabcCurBlRead(struct cabc_para *cabc_para) {
    int ret;
    int fd;
    uint16_t cur_bl;
    if (cabc_para != NULL) {
        fd = open(FILE_CABC_CUR_BL, O_RDONLY);
        if (fd >= 0) {
            ret =read(fd, &cur_bl, sizeof(uint16_t));
            cabc_para->cur_bl = cur_bl;
            close(fd);
        } else {
            ALOGE("%s: open %s failed\n", __func__, FILE_CABC_CUR_BL);
            ret = -1;
        }
    }
    else {
        ALOGE("%s: pointer cabc_para is null\n", __func__);
        ret = -1;
    }
    return (ret < 0) ? -errno : 0;
}

int CABC::resolutionRead(int *hactive, int *vactive) {
    int ret;
    int fd;
    char resolution[12] = {};
    char *resol;
    if((hactive != NULL) && (vactive != NULL)) {
        fd = open(FILE_RESOLUTION, O_RDONLY);
        if (fd >= 0) {
            ret =read(fd, resolution, sizeof(resolution));
            close(fd);
        } else {
            ALOGE("%s: open %s failed\n", __func__, FILE_RESOLUTION);
            ret = -1;
        }
        resol = strtok(resolution, "x");
        ret = atoi(resol);
        if (ret > 0)
            *hactive = ret;
        else {
            ret = -1;
            ALOGE("%s: atoi function works failed\n", __func__);
        }
        resol = strtok(NULL, "x");
        ret = atoi(resol);
        if (ret > 0)
            *vactive = ret;
        else {
            ret = -1;
            ALOGE("%s: atoi function works failed\n", __func__);
        }
    }
    else {
        ALOGE("%s: pointer hactive or vactive is null\n", __func__);
        ret = -1;
    }
    return (ret < 0) ? -errno : 0;
}

int CABC::vsyncCountRead(int *vsync_count) {
    int ret;
    int fd;
    if (vsync_count !=NULL) {
        fd = open(FILE_VSYNC_COUNT, O_RDONLY);
        if (fd >= 0) {
            ret =read(fd, vsync_count, sizeof(int));
            close(fd);
        } else {
            ALOGE("%s: open %s failed\n", __func__, FILE_VSYNC_COUNT);
            ret = -1;
        }
    }
    else {
        ALOGE("%s: pointer vsync_count is null\n", __func__);
        ret = -1;
    }
    return (ret < 0) ? -errno : 0;
}

int CABC::frameNoRead(int *frame_no) {
    int ret;
    int fd;
    if (frame_no != NULL) {
        fd = open(FILE_FRAME_NO, O_RDONLY);
        if (fd >= 0) {
            ret =read(fd, frame_no, sizeof(int));
            close(fd);
        } else {
            ALOGE("%s: open %s failed\n", __func__, FILE_FRAME_NO);
            ret = -1;
        }
    }
    else {
        ALOGE("%s: pointer frame_no is null\n", __func__);
        ret = -1;
    }
    return (ret < 0) ? -errno : 0;
}

int CABC::cabcSetGain(struct cabc_para *cabc_para) {
    int ret;
    int fd;
    uint16_t gain = 0;
    if (cabc_para != NULL) {
        gain = cabc_para->gain;
        fd = open(FILE_CABC_GAIN, O_WRONLY);
        if (fd >= 0) {
            ret =write(fd, &gain, sizeof(uint16_t));
            close(fd);
        } else {
            ALOGE("%s: open %s failed\n", __func__, FILE_CABC_GAIN);
            ret = -1;
        }
    }
    else {
        ALOGE("%s: pointer cabc_para is null\n", __func__);
        ret = -1;
    }
    return (ret < 0) ? -errno : 0;
}

int CABC::cabcSetBlfix(struct cabc_para *cabc_para) {
    int ret;
    int fd;
    uint16_t bl_fix = 0;
    if (cabc_para != NULL) {
        bl_fix = cabc_para->bl_fix;
        fd = open(FILE_CABC_BL_FIX, O_WRONLY);
        if (fd >= 0) {
            ret =write(fd, &bl_fix, sizeof(uint16_t));
            close(fd);
        } else {
            ALOGE("%s: open %s failed\n", __func__, FILE_CABC_BL_FIX);
            ret = -1;
        }
    }
    else {
        ALOGE("%s: pointer cabc_para is null\n", __func__);
        ret = -1;
    }
    return (ret < 0) ? -errno : 0;
}

int CABC::cabcSetRun(void) {
    int ret;
    int fd;
    uint16_t run_flag = 0;
    fd = open(FILE_CABC_RUN, O_WRONLY);
    if (fd >= 0) {
        ret =write(fd, &run_flag, sizeof(uint16_t));
        close(fd);
    } else {
        ALOGE("%s: open %s failed\n", __func__, FILE_CABC_RUN);
        ret = -1;
    }

    return (ret < 0) ? -errno : 0;
}

int CABC::cabcTrigger(void) {
    int i;
    static uint32_t cabc_hist [16]= {0};
    cabcHistRead(cabc_hist);
    for (i=0; i<16; i++) {
        cabc_para.cabc_hist[i] = cabc_hist[i];
    }
    /*
    *run cabc algorithm get the value of gain and bl_fix
    *that send to kernel layer
    */
    step_set(cabc_step0, cabc_step1, cabc_step2, cabc_step3,
            cabc_scene_change_thr, cabc_percent_thr_u,
            cabc_percent_thr_v, cabc_min_backlight);
    cabc_trigger(&cabc_para, cabc_no);
    cabcSetGain(&cabc_para);
    cabcSetBlfix(&cabc_para);
    return 0;
}

int CABC::cabcStart(void) {
    int ret = 0;
    int fd;
    int hactive = 0;
    int vactive = 0;
    if (init_flag) {
        resolutionRead(&hactive, &vactive);
        cabcInit(vactive, hactive);
        init_flag = 0;
    }
    /*
     *frame no keep in sync with kernel layer,
     *reading character type by module_param
     *which need to be tranfered into intergal
    */
    frameNoRead(&frame_no);
    fd = open(FILE_CABC_ENABLE, O_RDWR);
    if (fd >= 0) {
        ret = read(fd, &cabc_switch, sizeof(int));
        close (fd);
        } else {
                ALOGE("%s: open  %s failed\n", __func__, FILE_CABC_ENABLE);
                return 0;
               }
    return 0;
}

int CABC::cabcVsyncRun(void) {
    int hactive, vactive, vsync_count = 0;
    cabcStart();
    vsyncCountRead(&vsync_count);
    if ((cabc_switch != CABC_DISABLED) && (vsync_count >= 9) &&
        (vsync_count % 2 == 0) && (vsync_count < 151)) {
        cabcCurBlRead(&cabc_para);
        if (cabc_switch == CABC_STOPPING) {
            last_bl = cabc_para.cur_bl;
            cabc_no = 0;
            cabcSetRun();
            return 0;
        }
        if (!cabc_no) {
            resolutionRead(&hactive, &vactive);
            cabcInit(vactive, hactive);
            cabc_para.cur_bl = last_bl;
        }
        /* whether frame_no is 0 or not, run kernel shedule */
        if (frame_no == 0) {
            cabcSetRun();
            return 0;
        }
        else {
            if (cabc_no != 20)
                cabc_no ++;

            cabcTrigger();
            cabcSetRun();
            return 0;
       }
    }
    return 0;
}

int CABC::cabcSwitchMode(void) {
    if (cabc_switch == 0) {
        cabc_para.gain = 0x0400;
        cabc_para.bl_fix = 1020;
        cabcSetGain(&cabc_para);
        cabcSetBlfix(&cabc_para);
        cabcSetRun();
        cabc_flag = 1;
    }
    return 0;
}

int CABC::cabcFlipRun(void) {
    int hactive, vactive = 0;
    if (cabc_para.video_mode != 1)
        return 0;

    cabcStart();
    cabcCurBlRead(&cabc_para);
    if(cabc_switch == CABC_DISABLED)
        return 0;

    if (cabc_switch == CABC_STOPPING) {
        last_bl = cabc_para.cur_bl;
        cabc_no = 0;
        cabcSetRun();
        return 0;
    }
    if ((!cabc_no) || cabc_flag) {
        resolutionRead(&hactive, &vactive);
        cabcInit(vactive, hactive);
        cabc_para.cur_bl = last_bl;
        cabc_flag = 0;
    }
    /* whether frame_no is 0 or not, run kernel shedule */
    if (frame_no == 0) {
        cabcSetRun();
        return 0;
    }
    else {
        if (cabc_no != 20)
            cabc_no ++;
        cabcTrigger();
        cabcSetRun();
        return 0;
    }
    return 0;
}
