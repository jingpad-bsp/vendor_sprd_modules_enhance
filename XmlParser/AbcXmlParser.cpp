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

int AbcXmlParser::parseXmlData(const char *path) {
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
                } else if (!strcmp(elem->Name(), "slp_config")) {
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

int AbcXmlParser::parseAbcMode(tinyxml2::XMLElement *node, int *mode) {
    const tinyxml2::XMLAttribute *attribute;

    attribute = node->FirstAttribute();
    if (attribute && !strcmp(attribute->Name(), "mode")) {
        if (!strcmp(attribute->Value(), "normal"))
            *mode = 0;
        else if (!strcmp(attribute->Value(), "low power"))
            *mode = 1;
        else {
            ALOGE("%s invalid mode\n", __func__);
            return -1;
        }

        ALOGV("<mode=%s>", attribute->Value());
        return 0;
    }

    ALOGE("%s no mode\n", __func__);
    return -1;
}

int AbcXmlParser::parseAbcIndex(tinyxml2::XMLElement *node, int *index) {
    const tinyxml2::XMLAttribute *attribute;
    unsigned long int val;

    attribute = node->FirstAttribute();
    if (attribute && !strcmp(attribute->Name(), "index")) {
        val = strtoul(attribute->Value(), NULL, 0);
        *index = val;

        ALOGV("<number index=%s>", attribute->Value());
        return 0;
    }

    ALOGE("%s no index\n", __func__);
    return -1;
}

int AbcXmlParser::parseAbcEpfConfig(tinyxml2::XMLElement *node) {
    tinyxml2::XMLElement *epf_elem;
    tinyxml2::XMLElement *mode_elem = NULL;
    tinyxml2::XMLElement *elem;
    struct epf_cfg (*epf)[6] = abc->epf_slp;
    int mode, index, ret = -1;

    epf_elem = node->FirstChildElement();
    while (epf_elem) {
        if (!strcmp(epf_elem->Value(), "super_resolution")) {
            /* Look at tags: <super_resolution> */
            abc->sr_support = 1;
            epf = abc->epf_sr;
            mode_elem = epf_elem->FirstChildElement("config");
        } else if (!strcmp(epf_elem->Value(), "sunlight_protector")) {
            /* Look at tags: <sunlight_protector> */
            epf = abc->epf_slp;
            mode_elem = epf_elem->FirstChildElement("config");
        } else if (!strcmp(epf_elem->Value(), "config")) {
            mode_elem = epf_elem;
            epf_elem = NULL;
        }

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

                mEpfData.mEpfPtr = &epf[mode][index];
                ret = mEpfData.parseRegisterValue(elem);
                if (ret)
                    return ret;

                elem = elem->NextSiblingElement("number");
            }

            mode_elem = mode_elem->NextSiblingElement("config");
        }

        if (epf_elem)
            epf_elem = epf_elem->NextSiblingElement();
    }

    return ret;
}

int AbcXmlParser::parseAbcSlpConfig(tinyxml2::XMLElement *node) {
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

            mSlpData.mSlpPtr = &abc->slp[mode][index];
            ret = mSlpData.parseRegisterValue(elem);
            if (ret)
                return ret;

            elem = elem->NextSiblingElement("number");
        }

        mode_elem = mode_elem->NextSiblingElement("config");
    }

    return ret;
}

int AbcXmlParser::parseAbcSlpMapTable(tinyxml2::XMLElement *node) {
    tinyxml2::XMLElement *mode_elem;
    tinyxml2::XMLElement *elem;
    int mode, index, ret = -1;

    mode_elem = node->FirstChildElement("table");
    while (mode_elem) {
        /* Look at tags: <table mode="xxx"/> */
        ret = parseAbcMode(mode_elem, &mode);
        if (ret)
            return ret;

        elem = mode_elem->FirstChildElement("number");
        while (elem) {
            /* Look at tags: <number index="xxx"/> */
            ret = parseAbcIndex(elem, &index);
            if (ret)
                return ret;

            mSlpAmbientMapData.i = 0;
            mSlpAmbientMapData.mMapPtr = &abc->map[mode][index];
            ret = mSlpAmbientMapData.parseMapTable(elem);
            if (ret)
                return ret;

            elem = elem->NextSiblingElement("number");
        }

        mode_elem = mode_elem->NextSiblingElement("table");
    }

    return ret;
}
