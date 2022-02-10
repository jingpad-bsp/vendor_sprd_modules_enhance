#ifndef _DPU_LITE_R2P0_XML_PARSER_H_
#define _DPU_LITE_R2P0_XML_PARSER_H_

//#define LOG_NDEBUG 0

#include "EnhanceModule.h"
#include "XmlParser.h"

class RgbRangeData {
public:
    struct rgb_range *mRgbRangePtr;

    RgbRangeData() : mRgbRangePtr(NULL) { }
    virtual int parseRgbRange(tinyxml2::XMLElement *node);
    virtual ~RgbRangeData() { }
};

class DpuLiteR2P0BldXmlParser : public BldXmlParser {
public:
    struct dpu_lite_r2p0_bld_context *mBld;
    RgbRangeData mRgbRangeData;

    DpuLiteR2P0BldXmlParser();
    virtual int parseXmlData(const char *path);
    virtual int parseBldCmConfig(tinyxml2::XMLElement *node);
    virtual ~DpuLiteR2P0BldXmlParser() { };
};

class SlpLtmData {
public:
    struct dpu_lite_r2p0_slp_ltm_cfg *mSlpLtmPtr;

    SlpLtmData() : mSlpLtmPtr(NULL) { }
    virtual int parseRegisterValue(tinyxml2::XMLElement *node);
    virtual ~SlpLtmData() { }
};

class DpuLiteR2P0AbcXmlParser : public AbcXmlParser {
public:
    struct dpu_lite_r2p0_abc_context *mAbc;
    SlpLtmData mSlpLtmData;

    DpuLiteR2P0AbcXmlParser();
    virtual int parseXmlData(const char *path);
    virtual int parseAbcSlpConfig(tinyxml2::XMLElement *node);
    virtual ~DpuLiteR2P0AbcXmlParser() { }
};

class DpuLiteR2P0CmsXmlParser : public CmsXmlParser {
public:
    struct dpu_lite_r2p0_cms_context *mCms;
    SlpLtmData mSlpLtmData;
    EpfData mEpfData;

    DpuLiteR2P0CmsXmlParser();
    virtual int parseXmlData(const char *path);
    virtual int parseCmsHsvCmEpfSlpLtm(tinyxml2::XMLElement *node);
    virtual ~DpuLiteR2P0CmsXmlParser() { }
};

#endif

