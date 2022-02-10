/*
 *Copyright (C) 2019 Spreadtrum Communications Inc.
 *
 *This software is licensed under the terms of the GNU General Public
 *License version 2, as published by the Free Software Foundation, and
 *may be copied, distributed, and modified under those terms.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 */

#ifndef _CABC_DEFINITION_H_
#define _CABC_DEFINITION_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <linux/string.h>
#include <linux/kernel.h>
#include <stdbool.h>
#define STEP2_INIT   2
#define HIST_BIN_CABC  16

#define PRE  0
#define PRE_PRE 1
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

struct bl_out_tag {
	u16 cur;
	u16 pre;
	u16 pre2;
	u16 pre3;
	int cur_fix_ui;
	u16 pre_fix_ui;
	int cur_fix_video;
	u16 pre_fix_video;
};

struct cabc_context_tag {
	int width;
	int height;
	int pixel_total_num;
};

struct cabc_para {
	u32 cabc_hist[16];
	u16 gain;
	u16 bl_fix;
	u16 cur_bl;
	u8 video_mode;
};

int cabc_trigger(struct cabc_para *para, int frame_no);
void init_cabc(int img_width, int img_height);
void step_set(int step0, int step1, int step2, int step3,
	int scene_change_thr, int cabc_percent_thr_u,
	int cabc_percent_thr_v, int min_backlight);

#ifdef __cplusplus
}
#endif
#endif
