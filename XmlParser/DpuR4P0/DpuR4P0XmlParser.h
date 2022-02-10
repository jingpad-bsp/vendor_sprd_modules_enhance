#ifndef _DPU_R4P0_XML_PARSER_H_
#define _DPU_R4P0_XML_PARSER_H_

//#define LOG_NDEBUG 0

#include "EnhanceModule.h"
#include "XmlParser.h"
#include "DpuLiteR2P0XmlParser.h"


class DpuR4P0BldXmlParser : public DpuLiteR2P0BldXmlParser {
public:
    struct dpu_lite_r2p0_bld_context *mBld;

    DpuR4P0BldXmlParser() { mBld = (struct dpu_lite_r2p0_bld_context *)bld; }
    virtual ~DpuR4P0BldXmlParser(){};
};

class DpuR4P0SlpLtmData {
public:
    struct dpu_r4p0_slp_ltm_cfg *mSlpLtmPtr;

    DpuR4P0SlpLtmData() : mSlpLtmPtr(NULL) {}
    virtual int parseAbcRegisterSceneValue(tinyxml2::XMLElement *node);
    virtual int parseAbcRegisterModelValue(tinyxml2::XMLElement *node);
    virtual ~DpuR4P0SlpLtmData() {}
};

class DpuR4P0AbcXmlParser : public DpuLiteR2P0AbcXmlParser {
public:
    struct dpu_r4p0_abc_context *mAbc;
    DpuR4P0SlpLtmData mSlpLtmData;

    virtual int parseAbcSlpConfig(tinyxml2::XMLElement *node);
    DpuR4P0AbcXmlParser() { mAbc = (struct dpu_r4p0_abc_context *)abc; }
    virtual ~DpuR4P0AbcXmlParser() {}
};

class Lut3dData {
public:
    struct threed_lut *mLut3dPtr;
    int num;

    Lut3dData() : mLut3dPtr(NULL), num(0) { }
    virtual int parseRegisterValue(tinyxml2::XMLElement *node);
    virtual ~Lut3dData() { }
};

class DpuR4P0CmsXmlParser : public DpuLiteR2P0CmsXmlParser {
public:
    struct dpu_r4p0_cms_context *mCms;
    DpuR4P0SlpLtmData mSlpLtmData;
    Lut3dData mLut3dData;

    DpuR4P0CmsXmlParser();
    virtual int parseXmlData(const char *path);
    virtual int parseCmsHsvCmEpfSlpLtm(tinyxml2::XMLElement *node);
    virtual int parseCmsLut3dConfig(tinyxml2::XMLElement *node);
    virtual ~DpuR4P0CmsXmlParser() {}
};

#endif
