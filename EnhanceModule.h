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
#ifndef _ENHANCE_MODULE_H_
#define _ENHANCE_MODULE_H_

#include <log/log.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <hardware/enhance.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "enhanceHAL"

#define VENDOR_FOLDER "/vendor/etc/enhance/"
#define PROD_FOLDER "/mnt/vendor/enhance/"

#define FILE_CM        "/sys/class/display/dispc0/PQ/cm"
#define FILE_GAMMA    "/sys/class/display/dispc0/PQ/gamma"
#define FILE_HSV    "/sys/class/display/dispc0/PQ/hsv"
#define FILE_SLP    "/sys/class/display/dispc0/PQ/slp"
#define FILE_EPF    "/sys/class/display/dispc0/PQ/epf"
#define FILE_LTM    "/sys/class/display/dispc0/PQ/ltm"
#define FILE_LUT3D    "/sys/class/display/dispc0/PQ/lut3d"
#define FILE_DISABLE    "/sys/class/display/dispc0/PQ/disable"
#define FILE_STATUS    "/sys/class/display/dispc0/PQ/status"
#define FILE_DPU_VERSION "/sys/class/display/dispc0/dpu_version"
#define FILE_PANEL_NAME "/sys/class/display/panel0/name"
#define FILE_SR_EPF "/sys/class/display/dispc0/PQ/sr_epf"
#define FILE_CABC_MODE  "/sys/class/display/dispc0/PQ/cabc_mode"
#define FILE_CABC_ENABLE "/sys/class/display/dispc0/PQ/cabc_disable"

#define ABC_MODE_DISABLE            0
#define ABC_MODE_NORMAL             1
#define ABC_MODE_LOW_POWER      (1 << 1)
#define ABC_MODE_UI             (1 << 2)
#define ABC_MODE_GAME           (1 << 3)
#define ABC_MODE_VIDEO          (1 << 4)
#define ABC_MODE_IMAGE          (1 << 5)
#define ABC_MODE_CAMERA         (1 << 6)
#define ABC_MODE_FULL_FRAME     (1 << 7)

#define CABC_MODE_UI            (1 << 2)
#define CABC_MODE_GAME          (1 << 3)
#define CABC_MODE_VIDEO         (1 << 4)
#define CABC_MODE_IMAGE         (1 << 5)
#define CABC_MODE_CAMERA        (1 << 6)
#define CABC_MODE_FULL_FRAME    (1 << 7)
#define CABC_BRIGHTNESS_THRESHOLD 0x10
#define SLP_BRIGHTNESS_THRESHOLD 0x20
#define SLP_AMBIENT_REDUCTION 500

#define PQ_CMS 0x1
#define PQ_BLP 0x2
#define PQ_ABC 0x4
#define PQ_GAM 0x8
#define PQ_FLASH 0x10
#define PQ_CABC 0x20

#define MAX_XML_PATH_LENGTH 276

#define CABC_VSYNC 0XAABBCC
#define CABC_FLIP  0XCCAABB
#define CABC_STOPPING 1
#define CABC_DISABLED 2

enum {
    CABC_DISABLE = 0,
    CABC_ENABLE = 1
};

enum {
    CMS_MODE_DISABLE,
    CMS_MODE_AUTO,
    CMS_MODE_ENHANCE,
    CMS_MODE_STANDARD
};

enum {
    CMS_TEMP_NATURE = 0xFF000000,
    CMS_TEMP_WARM,
    CMS_TEMP_COLD
};

enum {
    BLD_MODE_DISABLE,
    BLD_MODE_HIGH,
    BLD_MODE_MIDDLE,
    BLD_MODE_DEFAULT,
    BLD_MODE_MAX = 64
};

enum {
    GAMMA_MODE_DISABLE,
    GAMMA_MODE_DEFAULT,
    GAMMA_MODE_STANDARD
};

#define ENHANCE_EPF             (1 << 1)
#define ENHANCE_HSV             (1 << 2)
#define ENHANCE_CM              (1 << 3)
#define ENHANCE_SLP             (1 << 4)
#define ENHANCE_GAMMA           (1 << 5)
#define ENHANCE_LTM             (1 << 6)
#define ENHANCE_LUT3D           (1 << 9)
#define ENHANCE_DITHER          (1 << 10)

struct cm_cfg {
    short coef00;
    short coef01;
    short coef02;
    short coef03;
    short coef10;
    short coef11;
    short coef12;
    short coef13;
    short coef20;
    short coef21;
    short coef22;
    short coef23;
};

struct hsv_lut {
    struct {
        uint16_t hue;
        uint16_t sat;
    } table[360];
};

struct gamma_lut {
    uint16_t r[256];
    uint16_t g[256];
    uint16_t b[256];
};

struct slp_cfg {
    uint8_t brightness;
    uint8_t conversion_matrix;
    uint8_t brightness_step;
    uint8_t second_bright_factor;
    uint8_t first_percent_th;
    uint8_t first_max_bright_th;
};

struct epf_cfg {
    uint16_t epsilon0;
    uint16_t epsilon1;
    uint8_t gain0;
    uint8_t gain1;
    uint8_t gain2;
    uint8_t gain3;
    uint8_t gain4;
    uint8_t gain5;
    uint8_t gain6;
    uint8_t gain7;
    uint8_t max_diff;
    uint8_t min_diff;
};

struct amb_item {
  uint16_t ambient;
  uint16_t brightness;
};

struct amb_mapping {
    struct amb_item item[128];
};

struct rgb_mapping {
    uint8_t rgb;
    uint8_t index;
};

struct hsv_cm {
    struct hsv_lut hsv;
    struct cm_cfg cm;
};

/*rmin/rmax/gmin/gmax/bmin/bmax:[0~1023]*/
struct rgb_range {
    uint16_t rmin;
    uint16_t rmax;
    uint16_t gmin;
    uint16_t gmax;
    uint16_t bmin;
    uint16_t bmax;
};

struct bld_context {
    uint8_t mode; // 0:disable; 1:high; 2:middle; 3:default
    struct hsv_cm hsvcm[3];
};

struct cms_context {
    uint8_t mode;    // 0:disable; 1:auto; 2:enhance; 3:standard
    uint32_t temp;   // 0xFF000000:nature; 0xFF000001:warm; 0xFF000002:cold; 0x00000000~0xFFFFFF:rgb value
    struct hsv_cm hsvcm[3];
    struct cm_cfg cm_manual[3];
    struct cm_cfg cm_auto[10];
    struct rgb_mapping map[10];
};

struct abc_context {
    uint8_t mode;                // 0:disable; 1:lowpower; 2:normal
    uint8_t index;
    int ambient;
    uint8_t sr_support;
    struct slp_cfg slp[2][6];
    struct amb_mapping map[2][6];
    struct epf_cfg epf_slp[2][6];
    struct epf_cfg epf_sr[2][6];
};

struct gamma_context {
    uint8_t mode;               //0:disable; 1:default; 2:standard
    uint8_t num_modes;
    struct gamma_lut lut[2];
};

struct flash_context {
    int is_enabled;
    int has_backup;
    int temp[16];
    struct cm_cfg cm[16];
    struct cm_cfg cm_backup;
};

struct dpu_lite_r2p0_bld_context {
    uint8_t mode;
    struct rgb_range rgb;
    struct cm_cfg cm;
};

struct dpu_lite_r2p0_slp_ltm_cfg {
    uint8_t brightness;
    uint8_t conversion_matrix;
    uint8_t brightness_step;
    uint8_t second_bright_factor;
    uint8_t first_percent_th;
    uint8_t first_max_bright_th;
    uint16_t low_clip;
    uint16_t high_clip;
    uint16_t step_clip;
    uint16_t mask_height;
    uint16_t dummy;
};

struct dpu_lite_r2p0_abc_context : public abc_context {
    struct dpu_lite_r2p0_slp_ltm_cfg slp_ltm[2][6];
};

struct dpu_lite_r2p0_cms_context : public cms_context {
    struct epf_cfg epf[3];
    struct dpu_lite_r2p0_slp_ltm_cfg slp_ltm[3];
};

struct dpu_r4p0_slp_ltm_cfg {
    uint8_t brightness;
    uint16_t brightness_step;
    uint8_t first_max_bright_th;
    uint8_t first_max_bright_th_step[5];
    uint8_t hist_exb_no;
    uint8_t hist_exb_percent;
    uint16_t mask_height;
    uint8_t first_pth_index[4];
    uint8_t hist9_index[9];
    uint8_t glb_x[3];
    uint16_t glb_s[3];
    uint16_t limit_hclip; // slp_high_clip
    uint16_t limit_lclip; // slp_low_clip
    uint16_t limit_clip_step; // slp_step_clip
    uint8_t fast_ambient_th;
    uint8_t scene_change_percent_th;
    uint8_t local_weight;
    uint8_t fst_pth; // first_percent_th
    uint8_t cabc_endv;
    uint8_t cabc_startv;
};

struct threed_lut {
    uint32_t value[729];
};

struct dpu_r4p0_abc_context : public abc_context {
    struct dpu_r4p0_slp_ltm_cfg slp_ltm[2][6];
};

struct dpu_r3p0_cabc_context{
    uint32_t user_mode;
    uint32_t kernel_mode;
};

struct dpu_r4p0_cms_context : public cms_context {
    struct epf_cfg epf[3];
    struct dpu_r4p0_slp_ltm_cfg slp_ltm[3];
    struct threed_lut lut3d[3];
    bool lut3d_support;
};

extern struct abc_context *abc;
extern struct bld_context *bld;
extern struct cms_context *cms;
extern struct gamma_context *gamma;
extern struct flash_context *flash;
extern struct dpu_cabc_context *cabc;
extern pthread_once_t g_init;
extern pthread_mutex_t g_lock;

void init_g_lock(void);
int get_dpu_version(char *version, int count);
int create_dpu_enhance_core(const char *version);
int get_enhance_param_from_xml(const char *id, const char *version);
int parse_xml_file(const char *version,
            const char *id, const char *path);
int abc_set_mode(int mode);
int abc_set_value(int value);
int bld_set_mode(int mode);
int cabc_set_mode(int mode);
int cms_set_mode(int mode);
int cms_set_value(int value);
int gamma_set_mode(int mode);

#endif
