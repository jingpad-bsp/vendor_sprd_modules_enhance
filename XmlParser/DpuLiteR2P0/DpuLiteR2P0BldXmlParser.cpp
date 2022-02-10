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

#include "DpuLiteR2P0XmlParser.h"

DpuLiteR2P0BldXmlParser::DpuLiteR2P0BldXmlParser() {
    mBld = (struct dpu_lite_r2p0_bld_context *)bld;
}

int DpuLiteR2P0BldXmlParser::parseXmlData(const char *path) {
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
                if (!strcmp(elem->Name(), "cm_config")) {
                    ret = parseBldCmConfig(elem);
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

int DpuLiteR2P0BldXmlParser::parseBldCmConfig(tinyxml2::XMLElement *node) {
    tinyxml2::XMLElement *elem;
    int ret = -1;

    elem = node->FirstChildElement("reg_table");
    if (elem)
        elem = elem->FirstChildElement();

    while (elem) {
        if (!strcmp(elem->Value(), "cm")) {
            mCmData.mCmPtr = &mBld->cm;
            ret = mCmData.parseRegisterValue(elem);
            if (ret)
                return ret;
        } else if (!strcmp(elem->Value(), "range")) {
            mRgbRangeData.mRgbRangePtr = &mBld->rgb;
            ret = mRgbRangeData.parseRgbRange(elem);
            if (ret)
                return ret;
        }

        elem = elem->NextSiblingElement();
    }

    return ret;
}

/* Look at tags: <param xxx="xxx"/> */
int RgbRangeData::parseRgbRange(tinyxml2::XMLElement *node) {
    const tinyxml2::XMLAttribute *attribute;
    tinyxml2::XMLElement *element;
    unsigned long int val;

    element = node->FirstChildElement("param");
    while (element) {
        attribute = element->FirstAttribute();
        if (attribute) {
            val = strtoul(attribute->Value(), NULL, 0);

            if (!strcmp(attribute->Name(), "rmin"))
                mRgbRangePtr->rmin = val;
            else if (!strcmp(attribute->Name(), "rmax"))
                mRgbRangePtr->rmax = val;
            else if (!strcmp(attribute->Name(), "gmin"))
                mRgbRangePtr->gmin = val;
            else if (!strcmp(attribute->Name(), "gmax"))
                mRgbRangePtr->gmax = val;
            else if (!strcmp(attribute->Name(), "bmin"))
                mRgbRangePtr->bmin = val;
            else if (!strcmp(attribute->Name(), "bmax"))
                mRgbRangePtr->bmax = val;

            ALOGV("\t<param %s=%lu/>", attribute->Name(), val);
        }

        element = element->NextSiblingElement("param");
    }

    return 0;
}
