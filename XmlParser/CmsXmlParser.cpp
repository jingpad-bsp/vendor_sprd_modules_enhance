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

#include "XmlParser.h"

int CmsXmlParser::parseXmlData(const char *path) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err;
    tinyxml2::XMLElement *elem;
    int ret = -1;

    err = doc.LoadFile(path);
    if (tinyxml2::XML_SUCCESS == err) {
        elem = doc.RootElement();
        if (elem) {
            elem = elem->FirstChildElement();
            while (elem) {
                if (!strcmp(elem->Name(), "hsv_cm")) {
                    ret = parseCmsHsvCm(elem);
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

int CmsXmlParser::parseCmsHsvCmMode(tinyxml2::XMLElement *node, int *mode) {
    const tinyxml2::XMLAttribute *attribute;

    attribute = node->FirstAttribute();
    if (attribute && !strcmp(attribute->Name(), "mode")) {
        if (!strcmp(attribute->Value(), "auto"))
            *mode = 0;
        else if (!strcmp(attribute->Value(), "enhance"))
            *mode = 1;
        else if (!strcmp(attribute->Value(), "standard"))
            *mode = 2;
        else {
            ALOGE("%s invalid mode\n", __func__);
            return -1;
        }

        ALOGV("<reg_table mode=%s>", attribute->Value());
        return 0;
    }

    ALOGE("%s no mode\n", __func__);
    return -1;
}

int CmsXmlParser::parseCmsHsvCm(tinyxml2::XMLElement *node) {
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
            }

            elem = elem->NextSiblingElement();
        }

        mode_elem = mode_elem->NextSiblingElement("reg_table");
    }
    /* The "nature" color temprature is the same of "auto" mode for cm HW. */
    memcpy(&cms->cm_manual[0], &cms->hsvcm[0].cm, sizeof(struct cm_cfg));

    return ret;
}

int CmsXmlParser::parseCmsRgbMap(tinyxml2::XMLElement *node) {
    int ret;

    mCmsRgbMapData.mMapPtr = cms->map;
    ret = mCmsRgbMapData.parseMapTable(node);
    return ret;
}

int CmsXmlParser::parseCmsCmCfgMode(tinyxml2::XMLElement *node) {
    const tinyxml2::XMLAttribute *attribute;
    static struct cm_cfg *pcm;
    int mode;

    attribute = node->FirstAttribute();
    if (attribute && !strcmp(attribute->Name(), "mode")) {
        if (!strcmp(attribute->Value(), "nature")) {
            mode = 0;
            pcm = cms->cm_manual;
        } else if (!strcmp(attribute->Value(), "warm")) {
            mode = 1;
            pcm = cms->cm_manual;
        } else if (!strcmp(attribute->Value(), "cold")) {
            mode = 2;
            pcm = cms->cm_manual;
        } else if (!strcmp(attribute->Value(), "0")) {
            mode = 0;
            pcm = cms->cm_auto;
        } else if (!strcmp(attribute->Value(), "1"))
            mode = 1;
        else if (!strcmp(attribute->Value(), "2"))
            mode = 2;
        else if (!strcmp(attribute->Value(), "3"))
            mode = 3;
        else if (!strcmp(attribute->Value(), "4"))
            mode = 4;
        else if (!strcmp(attribute->Value(), "5"))
            mode = 5;
        else if (!strcmp(attribute->Value(), "6"))
            mode = 6;
        else if (!strcmp(attribute->Value(), "7"))
            mode = 7;
        else if (!strcmp(attribute->Value(), "8"))
            mode = 8;
        else if (!strcmp(attribute->Value(), "9"))
            mode = 9;
        else {
            ALOGE("invalid reg_table mode: %s!", attribute->Value());
            return -1;
        }

        mCmData.mCmPtr = &pcm[mode];

        ALOGV("<reg_table mode=%s/>", attribute->Value());
        return 0;
    }

    ALOGE("%s no mode", __func__);
    return -1;
}

int CmsXmlParser::parseCmsCmCfg(tinyxml2::XMLElement *node) {
    tinyxml2::XMLElement *elem;
    int ret = -1;

    elem = node->FirstChildElement("reg_table");
    while (elem) {
        /* Look at tags: <reg_table mode="xxx"> */
        ret = parseCmsCmCfgMode(elem);
        if (ret)
            return ret;

        ret = mCmData.parseRegisterValue(elem);
        if (ret)
            return ret;

        elem = elem->NextSiblingElement("reg_table");
    }

    return ret;
}
