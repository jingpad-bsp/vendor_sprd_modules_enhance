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
#ifndef _DPU_ENHANCE_CORE_H_
#define _DPU_ENHANCE_CORE_H_

#include "EnhanceModule.h"
#include "Cabc.h"
// base class
class DpuEnhanceCore {
public:
    int mAbcEnable;
    int mCount;
    uint8_t mBrightness;

    DpuEnhanceCore();
    virtual int abcSetMode(int mode);
    virtual int slpSetBrightness(int ambient);
    virtual int cmsSetMode(int mode);
    virtual int cmsSetTemperature(int temp);
    virtual int flashSetTemperature(int temp);
    virtual int bldSetMode(int mode);
    virtual int gammaSetMode(int mode);
    int hsvGetValue(void);
    virtual int slpGetValue(void);
    int cmGetValue(void);
    virtual int gammaGetValue(void);
    int epfGetValue(void);
    virtual int slpRegRead(void *buf);
    int cmRegRead(void *buf);
    virtual int cabcSetMode(uint32_t mode);
    virtual int cabcGetValue(void);
    int hsvRegRead(void *buf);
    int gammaRegRead(void *buf);
    int epfRegRead(void *buf);
    uint32_t enhanceStatus(void);
    virtual ~DpuEnhanceCore();

protected:
    int enhanceDisable(uint32_t module);
    int ambientToBrightness(int ambient);
    int slpRegWrite(struct slp_cfg *slpPtr);
    int epfRegWrite(struct epf_cfg *epfPtr);
    int rgbToIndex(int rgb);
    int cmRegWrite(struct cm_cfg *cmPtr);
    int hsvRegWrite(struct hsv_lut *hsvPtr);
    int gammaRegWrite(struct gamma_lut *gammaPtr);
};

//dpu r2p0
class DpuR2P0EnhanceCore : public DpuEnhanceCore {
public:
    virtual int abcSetMode(int mode);
    virtual int slpSetBrightness(int ambient);
    virtual ~DpuR2P0EnhanceCore() { };
};

//dpu lite r2p0
class DpuLiteR2P0EnhanceCore : public DpuEnhanceCore {
public:
    struct dpu_lite_r2p0_bld_context *mBld;
    struct dpu_lite_r2p0_abc_context *mAbc;
    struct dpu_lite_r2p0_cms_context *mCms;

    DpuLiteR2P0EnhanceCore();
    virtual int cmsSetMode(int mode);
    virtual int bldSetMode(int mode);
    virtual int gammaSetMode(int mode);
    virtual int abcSetMode(int mode);
    virtual int slpSetBrightness(int ambient);
    virtual int slpGetValue(void);
    virtual int slpRegRead(void *buf);
    virtual ~DpuLiteR2P0EnhanceCore();

protected:
    int slpLtmRegWrite(struct dpu_lite_r2p0_slp_ltm_cfg *slpLtmPtr);
    int slpRegWrite(struct dpu_lite_r2p0_slp_ltm_cfg *slpLtmPtr);
    void levelToCoefficient(short *coef00, short *coef11, short *coef22);
};

//dpu r3p0
class DpuR3P0EnhanceCore : public DpuLiteR2P0EnhanceCore {
public:
    uint8_t lastBrightness;
    struct dpu_r3p0_cabc_context *mCabc;
    #ifdef SPRD_CABC
    CABC *cabc_ctr;
    #endif
    DpuR3P0EnhanceCore();
    virtual int cabcSetMode(uint32_t mode);
    virtual int cabcGetValue(void);
    virtual bool cabcIsEnable(int mode);
    virtual int cabcModeWrite(uint32_t mode);
    virtual int cabcEnable(uint32_t mode);
    virtual int cmsSetTemperature(int temp);
    virtual int cmsSetMode(int mode);
    virtual int bldSetMode(int mode);
    virtual int slpSetBrightness(int ambient);
    virtual int abcSetMode(int mode);
    virtual ~DpuR3P0EnhanceCore();
};

// dpu r4p0
class DpuR4P0EnhanceCore : public DpuLiteR2P0EnhanceCore {
public:
    struct dpu_r4p0_abc_context *mAbc;
    struct dpu_r4p0_cms_context *mCms;

    DpuR4P0EnhanceCore();
    virtual int slpRegRead(void *buf);
    virtual int slpGetValue(void);
    virtual ~DpuR4P0EnhanceCore();
    virtual int cmsSetMode(int mode);
    virtual int abcSetMode(int mode);
    virtual int gammaSetMode(int mode);
    virtual int slpSetBrightness(int ambient);
    virtual int gammaGetValue(void);
    virtual int lut3dRegRead(void *buf);

protected:
    int slpLtmRegWrite(struct dpu_r4p0_slp_ltm_cfg *slpLtmPtr);
    int lut3dRegWrite(struct threed_lut *lut3dPtr);
};

#endif
