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

#include "XmlData.h"

/* Look at tags: <param hue="xxx" sat="xxx"/> */
int HsvData::parseRegisterValue(tinyxml2::XMLElement *node) {
    const tinyxml2::XMLAttribute *attribute;
    tinyxml2::XMLElement *element;
    unsigned long int val;

    element = node->FirstChildElement("param");
    while (element) {
        attribute = element->FirstAttribute();
        while (attribute) {
            val = strtoul(attribute->Value(), NULL, 0);

            if (!strcmp("hue", attribute->Name()))
                mHsvPtr->table[i].hue = val;
            else if (!strcmp("sat", attribute->Name()))
                mHsvPtr->table[i].sat = val;

            attribute = attribute->Next();
        }

        ALOGV("\t<param hue=%hu sat=%hu/>", mHsvPtr->table[i].hue, mHsvPtr->table[i].sat);

        i++;
        element = element->NextSiblingElement("param");
    }

    return 0;
}

/* Look at tags: <param coefxx="xxx"/> */
int CmData::parseRegisterValue(tinyxml2::XMLElement *node) {
    const tinyxml2::XMLAttribute *attribute;
    tinyxml2::XMLElement *element;
    long int val;

    element = node->FirstChildElement("param");
    while (element) {
        attribute = element->FirstAttribute();
        if (attribute) {
            val = strtol(attribute->Value(), NULL, 0);

            ALOGV("\t<param %s=%ld/>", attribute->Name(), val);

            if (!strcmp(attribute->Name(), "coef00"))
                mCmPtr->coef00 = val;
            else  if (!strcmp(attribute->Name(), "coef01"))
                mCmPtr->coef01 = val;
            else  if (!strcmp(attribute->Name(), "coef02"))
                mCmPtr->coef02 = val;
            else  if (!strcmp(attribute->Name(), "coef03"))
                mCmPtr->coef03 = val;
            else  if (!strcmp(attribute->Name(), "coef10"))
                mCmPtr->coef10 = val;
            else  if (!strcmp(attribute->Name(), "coef11"))
                mCmPtr->coef11 = val;
            else  if (!strcmp(attribute->Name(), "coef12"))
                mCmPtr->coef12 = val;
            else  if (!strcmp(attribute->Name(), "coef13"))
                mCmPtr->coef13 = val;
            else  if (!strcmp(attribute->Name(), "coef20"))
                mCmPtr->coef20 = val;
            else  if (!strcmp(attribute->Name(), "coef21"))
                mCmPtr->coef21 = val;
            else  if (!strcmp(attribute->Name(), "coef22"))
                mCmPtr->coef22 = val;
            else  if (!strcmp(attribute->Name(), "coef23"))
                mCmPtr->coef23 = val;
        }

        element = element->NextSiblingElement("param");
    }

    return 0;
}

/* Look at tags: <param xxx="xxx"/> */
int SlpData::parseRegisterValue(tinyxml2::XMLElement *node) {
    const tinyxml2::XMLAttribute *attribute;
    tinyxml2::XMLElement *element;
    unsigned long int val;

    element = node->FirstChildElement("param");
    while (element) {
        attribute = element->FirstAttribute();
        if (attribute) {
            val = strtoul(attribute->Value(), NULL, 0);

            ALOGV("\t<param %s=%lu/>", attribute->Name(), val);

            if (!strcmp(attribute->Name(), "brightness"))
                mSlpPtr->brightness = val;
            else if (!strcmp(attribute->Name(), "conversion_matrix"))
                mSlpPtr->conversion_matrix = val;
            else if (!strcmp(attribute->Name(), "brightness_step"))
                mSlpPtr->brightness_step = val;
            else if (!strcmp(attribute->Name(), "second_bright_factor"))
                mSlpPtr->second_bright_factor = val;
            else if (!strcmp(attribute->Name(), "first_percent_th"))
                mSlpPtr->first_percent_th = val;
            else if (!strcmp(attribute->Name(), "first_max_bright_th"))
                mSlpPtr->first_max_bright_th = val;
        }

        element = element->NextSiblingElement("param");
    }

    return 0;
}

/* Look at tags: <param xxx="xxx"/> */
int EpfData::parseRegisterValue(tinyxml2::XMLElement *node) {
    const tinyxml2::XMLAttribute *attribute;
    tinyxml2::XMLElement *element;
    unsigned long int val;

    element = node->FirstChildElement("param");
    while (element) {
        attribute = element->FirstAttribute();
        if (attribute) {
            val = strtoul(attribute->Value(), NULL, 0);

            ALOGV("\t<param %s=%s/>", attribute->Name(), attribute->Value());

            if (!strcmp(attribute->Name(), "epsilon0"))
                mEpfPtr->epsilon0 = val;
            else if (!strcmp(attribute->Name(), "epsilon1"))
                mEpfPtr->epsilon1 = val;
            else if (!strcmp(attribute->Name(), "gain0"))
                mEpfPtr->gain0 = val;
            else if (!strcmp(attribute->Name(), "gain1"))
                mEpfPtr->gain1 = val;
            else if (!strcmp(attribute->Name(), "gain2"))
                mEpfPtr->gain2 = val;
            else if (!strcmp(attribute->Name(), "gain3"))
                mEpfPtr->gain3 = val;
            else if (!strcmp(attribute->Name(), "gain4"))
                mEpfPtr->gain4 = val;
            else if (!strcmp(attribute->Name(), "gain5"))
                mEpfPtr->gain5 = val;
            else if (!strcmp(attribute->Name(), "gain6"))
                mEpfPtr->gain6 = val;
            else if (!strcmp(attribute->Name(), "gain7"))
                mEpfPtr->gain7 = val;
            else if (!strcmp(attribute->Name(), "max_diff"))
                mEpfPtr->max_diff = val;
            else if (!strcmp(attribute->Name(), "min_diff"))
                mEpfPtr->min_diff = val;
        }

        element = element->NextSiblingElement("param");
    }

    return 0;
}

/* Look at tags: <param r="xxx" g="xxx" b="xxx"/>*/
int GammaData::parseRegisterValue(tinyxml2::XMLElement *node) {
    const tinyxml2::XMLAttribute *attribute;
    tinyxml2::XMLElement *element;
    unsigned long int val;

    element = node->FirstChildElement("param");
    while (element) {
        attribute = element->FirstAttribute();
        while (attribute) {
            val = strtoul(attribute->Value(), NULL, 0);

            if (!strcmp("r", attribute->Name()))
                mGammaPtr->r[i] = val;
            else if (!strcmp("g", attribute->Name()))
                mGammaPtr->g[i] = val;
            else if (!strcmp("b", attribute->Name()))
                mGammaPtr->b[i] = val;

            attribute = attribute->Next();
        }

        ALOGV("\t<param r=%u g=%u b=%u/>", mGammaPtr->r[i], mGammaPtr->g[i], mGammaPtr->b[i]);

        i++;
        element = element->NextSiblingElement("param");
    }

    return 0;
}

/* Look at tags: <item ambient="xxx" brightness="xxx"/> */
int SlpAmbientMapData::parseMapTable(tinyxml2::XMLElement *node) {
    const tinyxml2::XMLAttribute *attribute;
    tinyxml2::XMLElement *element;
    unsigned long int val;

    element = node->FirstChildElement("item");
    while (element) {
        attribute = element->FirstAttribute();
        while (attribute) {
            val = strtoul(attribute->Value(), NULL, 0);

            if (!strcmp(attribute->Name(), "ambient"))
                mMapPtr->item[i].ambient = val;
            else if (!strcmp(attribute->Name(), "brightness"))
                mMapPtr->item[i].brightness = val;

            attribute = attribute->Next();
        }

        ALOGV("\t<item ambient=%u brightness=%u/>",
                mMapPtr->item[i].ambient, mMapPtr->item[i].brightness);

        i++;
        element = element->NextSiblingElement("item");
    }

    return 0;
}

/* Look at tags: <item rgb="xxx" index="xxx"/> */
int CmsRgbMapData::parseMapTable(tinyxml2::XMLElement *node) {
    const tinyxml2::XMLAttribute *attribute;
    tinyxml2::XMLElement *element;
    unsigned long int val;

    element = node->FirstChildElement("item");
    while (element) {
        attribute = element->FirstAttribute();
        while (attribute) {
            val = strtoul(attribute->Value(), NULL, 0);

            if (!strcmp("rgb", attribute->Name()))
                mMapPtr->rgb = val;
            else if (!strcmp("index", attribute->Name()))
                mMapPtr->index = val;

            attribute = attribute->Next();
        }

        ALOGV("\t<item rgb=%u index=%u/>", mMapPtr->rgb, mMapPtr->index);

        mMapPtr++;
        element = element->NextSiblingElement("item");
    }

    return 0;
}
