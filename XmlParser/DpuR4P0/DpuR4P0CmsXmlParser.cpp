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

#include "DpuR4P0XmlParser.h"

DpuR4P0CmsXmlParser::DpuR4P0CmsXmlParser() {
    mCms = (struct dpu_r4p0_cms_context *)cms;
}

int DpuR4P0CmsXmlParser::parseXmlData(const char *path) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err;
    tinyxml2::XMLElement *elem;
    int ret = -1;

    err = doc.LoadFile(path);
    if (tinyxml2::XML_SUCCESS == err) {
        elem = doc.RootElement();
        if (elem) {
            if (!strcmp(elem->Name(), "LUT_config")) {
                mCms->lut3d_support = true;
                ret = parseCmsLut3dConfig(elem);
                if (ret)
                    mCms->lut3d_support = false;

                return ret;
            }

            elem = elem->FirstChildElement();
            while (elem) {
                if (!strcmp(elem->Name(), "hsv_cm_epf_slp_ltm")) {
                    ret = parseCmsHsvCmEpfSlpLtm(elem);
                    if (ret)
                        return ret;
                } else if (!strcmp(elem->Name(), "rgb_mapping_table")) {
                    ret = parseCmsRgbMap(elem);
                    if (ret)
                        return ret;
                } else if (!strcmp(elem->Name(), "cm_cfg")) {
                    ret = parseCmsCmCfg(elem);
                    if (ret)
                        return ret;
                }

                elem = elem->NextSiblingElement();
            }
        }
    } else
        ALOGE("%s LoadFile file:%s failed\n", __func__, path);

    return ret;
}

int DpuR4P0CmsXmlParser::parseCmsHsvCmEpfSlpLtm(tinyxml2::XMLElement *node) {
    tinyxml2::XMLElement *mode_elem;
    tinyxml2::XMLElement *elem;
    int ret = -1;
    int mode;

    mode_elem = node->FirstChildElement("reg_table");
    while (mode_elem) {
        /* Look at tags: <reg_table mode="xxx"> */
        ret = parseCmsHsvCmMode(mode_elem, &mode);
        if (ret)
            return ret;

        elem = mode_elem->FirstChildElement();
        while (elem) {
             if (!strcmp(elem->Value(), "hsv")) {
                 /* Look at tags: <hsv> */
                 mHsvData.i = 0;
                 mHsvData.mHsvPtr = &cms->hsvcm[mode].hsv;
                 ret = mHsvData.parseRegisterValue(elem);
                 if (ret)
                     return ret;
             } else if (!strcmp(elem->Value(), "cm")) {
                 /* Look at tags: <cm> */
                 mCmData.mCmPtr = &cms->hsvcm[mode].cm;
                 ret = mCmData.parseRegisterValue(elem);
                 if (ret)
                     return ret;
             } else if (!strcmp(elem->Value(), "epf")) {
                 /* Look at tags: <epf> */
                 mEpfData.mEpfPtr = &mCms->epf[mode];
                 ret = mEpfData.parseRegisterValue(elem);
                 if (ret)
                     return ret;
             } else if (!strcmp(elem->Value(), "slp") ||
                !strcmp(elem->Value(), "ltm")) {
                 /* Look at tags: <slp> or <ltm> */
                 mSlpLtmData.mSlpLtmPtr = &mCms->slp_ltm[mode];
                 ret = mSlpLtmData.parseAbcRegisterSceneValue(elem);
                 if (ret)
                     return ret;
             }

             elem = elem->NextSiblingElement();
        }

        mode_elem = mode_elem->NextSiblingElement("reg_table");
    }
    /* The "nature" color temprature is the same of "auto" mode for cm HW. */
    memcpy(&cms->cm_manual[0], &cms->hsvcm[0].cm, sizeof(struct cm_cfg));

    return ret;
}

/* Look at tags: <param r="xxx" g="xxx" b="xxx"/> */
int Lut3dData::parseRegisterValue(tinyxml2::XMLElement *node) {
    const tinyxml2::XMLAttribute *attribute;
    tinyxml2::XMLElement *element;
    unsigned long int val;
    int i = 0;

    element = node->FirstChildElement("param");
    while (element && i < num) {
        attribute = element->FirstAttribute();
        while (attribute) {
            val = strtoul(attribute->Value(), NULL, 0);

            if (!strcmp("r", attribute->Name()))
                mLut3dPtr->value[i] |= val << 20;
            else if (!strcmp("g", attribute->Name()))
                mLut3dPtr->value[i] |= val << 10;
            else if (!strcmp("b", attribute->Name()))
                mLut3dPtr->value[i] |= val;

            attribute = attribute->Next();
            ALOGV("val:%d mLut3dPtr:0x%x", val, mLut3dPtr->value[i]);
        }

        i++;
        element = element->NextSiblingElement("param");
    }

    return 0;
}

int DpuR4P0CmsXmlParser::parseCmsLut3dConfig(tinyxml2::XMLElement *node) {
    tinyxml2::XMLElement *elem;
    int ret = -1;
    int mode;

    elem = node->FirstChildElement("reg_table");
    while (elem) {
        /* Look at tags: <reg_table mode="xxx"> */
        ret = parseCmsHsvCmMode(elem, &mode);
        if (ret)
            return ret;

        mLut3dData.num = sizeof(mCms->lut3d[mode]);
        mLut3dData.mLut3dPtr = &mCms->lut3d[mode];
        ret = mLut3dData.parseRegisterValue(elem);
        if (ret)
            return ret;

        elem = elem->NextSiblingElement("reg_table");
    }

    return ret;
}
