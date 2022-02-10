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
#ifndef _CABC_FUNCTION_H_
#define _CABC_FUNCTION_H_

#include <log/log.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "EnhanceModule.h"
#include "cabc_definition.h"

#define FILE_CABC_HIST "/sys/class/display/dispc0/PQ/cabc_hist"
#define FILE_CABC_GAIN "/sys/class/display/dispc0/PQ/cabc_gain"
#define FILE_CABC_BL_FIX "/sys/class/display/dispc0/PQ/cabc_bl_fix"
#define FILE_CABC_CUR_BL "/sys/class/display/dispc0/PQ/cabc_cur_bl"
#define FILE_VSYNC_COUNT "/sys/class/display/dispc0/PQ/vsync_count"
#define FILE_FRAME_NO "/sys/class/display/dispc0/PQ/frame_no"
#define FILE_CABC_RUN "/sys/class/display/dispc0/PQ/cabc_run"
#define FILE_FLIP_RUN "/sys/class/display/dispc0/PQ/flip_run"
#define FILE_RESOLUTION "/sys/class/display/panel0/resolution"

class CABC {
public:

    struct cabc_para cabc_para;
    int cabc_step0;
    int cabc_step1;
    int cabc_step2;
    int cabc_step3;
    int cabc_scene_change_thr;
    int cabc_percent_thr_u;
    int cabc_percent_thr_v;
    int cabc_min_backlight;
    int frame_no;
    int cabc_no;
    int init_flag;
    int last_bl;
    int cabc_switch;
    int cabc_flag;

    CABC();
    void cabcMode (uint32_t cabc_mode);
    int cabcStart(void);
    int cabcVsyncRun(void);
    int cabcFlipRun(void);
    int cabcTrigger(void);
    int cabcHistRead(uint32_t hist[32]);
    int cabcSetGain(struct cabc_para *cabc_para);
    int cabcSetBlfix(struct cabc_para *cabc_para);
    int cabcCurBlRead(struct cabc_para *cabc_para);
    int vsyncCountRead(int *vsync_count);
    int frameNoRead(int *frame_no);
    int cabcSetRun(void);
    int resolutionRead(int *hactive, int *vactive);
    int cabcSwitchMode(void);
    virtual ~CABC();
};

#endif
