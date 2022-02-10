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
#ifndef _XML_DATA_H_
#define _XML_DATA_H_

//#define LOG_NDEBUG 0

#include <log/log.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <tinyxml2.h>
#include "EnhanceModule.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "enhanceHAL"

class HsvData {
public:
    struct hsv_lut *mHsvPtr;
    int i;

    HsvData() : mHsvPtr(NULL), i(0) { }
    virtual int parseRegisterValue(tinyxml2::XMLElement *node);
    virtual ~HsvData() { }
};

class CmData {
public:
    struct cm_cfg *mCmPtr;

    CmData() : mCmPtr(NULL) { }
    virtual int parseRegisterValue(tinyxml2::XMLElement *node);
    virtual ~CmData() { }
};

class SlpData {
public:
    struct slp_cfg *mSlpPtr;

    SlpData() : mSlpPtr(NULL) { }
    virtual int parseRegisterValue(tinyxml2::XMLElement *node);
    virtual ~SlpData() { }
};

class EpfData {
public:
    struct epf_cfg *mEpfPtr;

    EpfData() : mEpfPtr(NULL) { }
    virtual int parseRegisterValue(tinyxml2::XMLElement *node);
    virtual ~EpfData() { }
};

class GammaData {
public:
    struct gamma_lut *mGammaPtr;
    int i;

    GammaData() : mGammaPtr(NULL), i(0) { }
    virtual int parseRegisterValue(tinyxml2::XMLElement *node);
    virtual ~GammaData() { }
};

class SlpAmbientMapData {
public:
    struct amb_mapping *mMapPtr;
    int i;

    SlpAmbientMapData() : mMapPtr(NULL), i(0) { }
    virtual int parseMapTable(tinyxml2::XMLElement *node);
    virtual ~SlpAmbientMapData() { }
};

class CmsRgbMapData {
public:
    struct rgb_mapping *mMapPtr;
    int i;

    CmsRgbMapData() : mMapPtr(NULL), i(0) { }
    virtual int parseMapTable(tinyxml2::XMLElement *node);
    virtual ~CmsRgbMapData() { }
};

#endif
