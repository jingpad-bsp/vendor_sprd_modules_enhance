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

int GammaXmlParser::parseXmlData(const char *path) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err;
    tinyxml2::XMLElement *elem;
    int ret = -1;

    err = doc.LoadFile(path);
    if (tinyxml2::XML_SUCCESS == err) {
        elem = doc.RootElement();
        if (elem) {
            if (!strcmp("root", elem->Value()))
                elem = elem->FirstChildElement();

            while (elem) {
                if (!strcmp(elem->Name(), "gamma_config")) {
                    ret = parseGammaConfig(elem);
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

int GammaXmlParser::parseGammaConfigMode(tinyxml2::XMLElement *node, int *mode) {
    const tinyxml2::XMLAttribute *attribute;

    attribute = node->FirstAttribute();
    if (attribute && !strcmp(attribute->Name(), "mode")) {
        if (!strcmp(attribute->Value(), "default"))
            *mode = 0;
        else if (!strcmp(attribute->Value(), "standard"))
            *mode = 1;
        else {
            ALOGE("%s invalid mode\n", __func__);
            return -1;
        }

        gamma->num_modes++;

        ALOGV("<reg_table mode=%s>", attribute->Value());
        return 0;
    }

    ALOGE("%s no mode\n", __func__);
    return -1;
}

int GammaXmlParser::parseGammaConfig(tinyxml2::XMLElement *node) {
    tinyxml2::XMLElement *elem;
    int mode, ret = -1;

    elem = node->FirstChildElement("reg_table");
    while (elem) {
        /* Look at tags: <reg_table mode="xxx"> */
        ret = parseGammaConfigMode(elem, &mode);
        if (ret)
            return ret;

        mGammaData.i = 0;
        mGammaData.mGammaPtr = &gamma->lut[mode];
        ret = mGammaData.parseRegisterValue(elem);
        if (ret)
            return ret;

        elem = elem->NextSiblingElement("reg_table");
    }

    return ret;
}
