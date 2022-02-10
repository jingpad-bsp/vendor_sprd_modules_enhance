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

DpuLiteR2P0AbcXmlParser::DpuLiteR2P0AbcXmlParser() {
    mAbc = (struct dpu_lite_r2p0_abc_context *)abc;
}

int DpuLiteR2P0AbcXmlParser::parseXmlData(const char *path) {
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
                if (!strcmp(elem->Name(), "epf_config")) {
                    ret = parseAbcEpfConfig(elem);
                    if (ret)
                        return ret;
                } else if (!strcmp(elem->Name(), "slp_config") ||
                    !strcmp(elem->Name(), "ltm_config")) {
                    ret = parseAbcSlpConfig(elem);
                    if (ret)
                        return ret;
                } else if (!strcmp(elem->Name(), "slp_mapping_table")) {
                    ret = parseAbcSlpMapTable(elem);
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

int DpuLiteR2P0AbcXmlParser::parseAbcSlpConfig(tinyxml2::XMLElement *node) {
    tinyxml2::XMLElement *mode_elem;
    tinyxml2::XMLElement *elem;
    int mode, index, ret = -1;

    mode_elem = node->FirstChildElement("config");
    while (mode_elem) {
        /* Look at tags: <config mode="xxx"/> */
        ret = parseAbcMode(mode_elem, &mode);
        if (ret)
            return ret;

        elem = mode_elem->FirstChildElement("number");
        while (elem) {
            /* Look at tags: <number index="xxx"/> */
            ret = parseAbcIndex(elem, &index);
            if (ret)
                return ret;

            mSlpLtmData.mSlpLtmPtr = &mAbc->slp_ltm[mode][index];
            ret = mSlpLtmData.parseRegisterValue(elem);
            if (ret)
                return ret;

            elem = elem->NextSiblingElement("number");
        }

        mode_elem = mode_elem->NextSiblingElement("config");
    }

    return ret;
}

/* Look at tags: <param xxx="xxx"/> */
int SlpLtmData::parseRegisterValue(tinyxml2::XMLElement *node) {
    const tinyxml2::XMLAttribute *attribute;
    tinyxml2::XMLElement *element;
    unsigned long int val;

    element = node->FirstChildElement("param");
    while (element) {
        attribute = element->FirstAttribute();
        if (attribute) {
            val = strtoul(attribute->Value(), NULL, 0);

            if (!strcmp(attribute->Name(), "brightness"))
                mSlpLtmPtr->brightness = val;
            else if (!strcmp(attribute->Name(), "conversion_matrix"))
                mSlpLtmPtr->conversion_matrix = val;
            else if (!strcmp(attribute->Name(), "brightness_step"))
                mSlpLtmPtr->brightness_step = val;
            else if (!strcmp(attribute->Name(), "second_bright_factor"))
                mSlpLtmPtr->second_bright_factor = val;
            else if (!strcmp(attribute->Name(), "first_percent_th"))
                mSlpLtmPtr->first_percent_th = val;
            else if (!strcmp(attribute->Name(), "first_max_bright_th"))
                mSlpLtmPtr->first_max_bright_th = val;
            else if (!strcmp(attribute->Name(), "slp_low_clip"))
                mSlpLtmPtr->low_clip = val;
            else if (!strcmp(attribute->Name(), "slp_high_clip"))
                mSlpLtmPtr->high_clip = val;
            else if (!strcmp(attribute->Name(), "slp_step_clip"))
                mSlpLtmPtr->step_clip = val;
            else if (!strcmp(attribute->Name(), "mask_height"))
                mSlpLtmPtr->mask_height = val;
            else if (!strcmp(attribute->Name(), "dummy"))
                mSlpLtmPtr->dummy = val;

            ALOGV("\t<param %s=%lu/>", attribute->Name(), val);
        }

        element = element->NextSiblingElement("param");
    }

    return 0;
}
