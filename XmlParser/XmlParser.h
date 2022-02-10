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
#ifndef _XML_PARSER_H_
#define _XML_PARSER_H_

//#define LOG_NDEBUG 0

#include "XmlData.h"

class VersionXmlParser {
public:
    virtual int enhanceFileCheck(const char *prod_file, const char *vendor_file);
    virtual ~VersionXmlParser() { }

protected:
    int parseVersion(const char *path, char (*version)[20]);
    int copy(const char *dst, const char *src);
};

//abc.xml
class AbcXmlParser {
public:
    SlpData mSlpData;
    EpfData mEpfData;
    SlpAmbientMapData mSlpAmbientMapData;

    virtual int parseXmlData(const char *path);
    int parseAbcMode(tinyxml2::XMLElement *node, int *mode);
    int parseAbcIndex(tinyxml2::XMLElement *node, int *index);
    virtual int parseAbcEpfConfig(tinyxml2::XMLElement *node);
    virtual int parseAbcSlpConfig(tinyxml2::XMLElement *node);
    virtual int parseAbcSlpMapTable(tinyxml2::XMLElement *node);
    virtual ~AbcXmlParser() { };
};

//bld xml
class BldXmlParser {
public:
    HsvData mHsvData;
    CmData mCmData;

    virtual int parseXmlData(const char *path);
    int parseBldHsvCmMode(tinyxml2::XMLElement *node, int *mode);
    virtual int parseBldHsvCm(tinyxml2::XMLElement *node);
    virtual ~BldXmlParser() { };
};

//cms xml
class CmsXmlParser {
public:
    HsvData mHsvData;
    CmData mCmData;
    CmsRgbMapData mCmsRgbMapData;

    virtual int parseXmlData(const char *path);
    virtual int parseCmsHsvCmMode(tinyxml2::XMLElement *node, int *mode);
    virtual int parseCmsHsvCm(tinyxml2::XMLElement *node);
    virtual int parseCmsCmCfgMode(tinyxml2::XMLElement *node);
    virtual int parseCmsCmCfg(tinyxml2::XMLElement *node);
    virtual int parseCmsRgbMap(tinyxml2::XMLElement *node);
    virtual ~CmsXmlParser() { };
};

//gamma xml
class GammaXmlParser {
public:
    GammaData mGammaData;

    virtual int parseXmlData(const char *path);
    virtual int parseGammaConfigMode(tinyxml2::XMLElement *node, int *mode);
    virtual int parseGammaConfig(tinyxml2::XMLElement *node);
    virtual ~GammaXmlParser() { };
};

//flash xml
class FlashXmlParser {
public:
    int mIndex;
    CmData mCmData;

    FlashXmlParser() : mIndex(0) { };
    virtual int parseXmlData(const char *path);
    virtual int parseColorTemperature(tinyxml2::XMLElement *node);
    virtual int parseFlashCmCfg(tinyxml2::XMLElement *node);
    virtual ~FlashXmlParser() { };
};

#endif
