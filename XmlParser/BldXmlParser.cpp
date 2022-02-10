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

int BldXmlParser::parseXmlData(const char *path) {
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
                    ret = parseBldHsvCm(elem);
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

int BldXmlParser::parseBldHsvCmMode(tinyxml2::XMLElement *node, int *mode) {
    const tinyxml2::XMLAttribute *attribute;

    attribute = node->FirstAttribute();
    if (attribute && !strcmp(attribute->Name(), "mode")) {
        if (!strcmp(attribute->Value(), "default"))
            *mode = 2;
        else if (!strcmp(attribute->Value(), "middle"))
            *mode = 1;
        else if (!strcmp(attribute->Value(), "high"))
            *mode = 0;
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

int BldXmlParser::parseBldHsvCm(tinyxml2::XMLElement *node) {
    tinyxml2::XMLElement *mode_elem;
    tinyxml2::XMLElement *elem;
    int ret = -1;
    int mode;

    mode_elem = node->FirstChildElement("reg_table");
    while (mode_elem) {
        /* Look at tags: <reg_table mode="xxx"> */
        ret = parseBldHsvCmMode(mode_elem, &mode);
        if (ret)
            return ret;

        elem = mode_elem->FirstChildElement();
        while (elem) {
            if (!strcmp(elem->Value(), "hsv")) {
                /* Look at tags: <hsv> */
                mHsvData.i = 0;
                mHsvData.mHsvPtr = &bld->hsvcm[mode].hsv;
                ret = mHsvData.parseRegisterValue(elem);
                if (ret)
                    return ret;
            } else if (!strcmp(elem->Value(), "cm")) {
                /* Look at tags: <cm> */
                mCmData.mCmPtr = &bld->hsvcm[mode].cm;
                ret = mCmData.parseRegisterValue(elem);
                if (ret)
                    return ret;
            }

            elem = elem->NextSiblingElement();
        }

        mode_elem = mode_elem->NextSiblingElement("reg_table");
    }

    return ret;
}
