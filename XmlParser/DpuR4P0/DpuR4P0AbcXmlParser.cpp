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

int DpuR4P0AbcXmlParser::parseAbcSlpConfig(tinyxml2::XMLElement *node) {
    tinyxml2::XMLElement *mode_elem;
    tinyxml2::XMLElement *elem;
    int mode, index, ret = -1;

    mode_elem = node->FirstChildElement("config");
    while (mode_elem) {
        /* Look at tags: <config mode="xxx"/> */
        ret = parseAbcMode(mode_elem, &mode);
        if (ret)
            return ret;

        elem = mode_elem->FirstChildElement("param");
        mSlpLtmData.mSlpLtmPtr = &mAbc->slp_ltm[mode][0];
        mSlpLtmData.parseAbcRegisterModelValue(elem);
        if (!strcmp(node->Name(), "slp_config")) {
            for (index = 0; index < 6; index++)
                memcpy(&mAbc->slp_ltm[mode][index], &mAbc->slp_ltm[mode][0],
                    sizeof(struct dpu_r4p0_slp_ltm_cfg));
        }

        elem = mode_elem->FirstChildElement("number");
        while (elem) {
            /* Look at tags: <number index="xxx"/> */
            ret = parseAbcIndex(elem, &index);
            if (ret)
                return ret;

            mSlpLtmData.mSlpLtmPtr = &mAbc->slp_ltm[mode][index];
            ret = mSlpLtmData.parseAbcRegisterSceneValue(elem);
            if (ret)
                return ret;

            elem = elem->NextSiblingElement("number");
        }

        mode_elem = mode_elem->NextSiblingElement("config");
    }

    return ret;
}

int DpuR4P0SlpLtmData::parseAbcRegisterModelValue(tinyxml2::XMLElement *node) {
    const tinyxml2::XMLAttribute *attribute;
    tinyxml2::XMLElement *element;
    unsigned long int val;

    element = node;
    while (element) {
        attribute = element->FirstAttribute();
        if (attribute) {
            val = strtoul(attribute->Value(), NULL, 0);

            if (!strcmp(attribute->Name(), "hist_exb_no"))
                mSlpLtmPtr->hist_exb_no = val;
            else if (!strcmp(attribute->Name(), "hist_exb_percent"))
                mSlpLtmPtr->hist_exb_percent = val;
            else if (!strcmp(attribute->Name(), "hist9_index0"))
                mSlpLtmPtr->hist9_index[0] = val;
            else if (!strcmp(attribute->Name(), "hist9_index1"))
                mSlpLtmPtr->hist9_index[1] = val;
            else if (!strcmp(attribute->Name(), "hist9_index2"))
                mSlpLtmPtr->hist9_index[2] = val;
            else if (!strcmp(attribute->Name(), "hist9_index3"))
                mSlpLtmPtr->hist9_index[3] = val;
            else if (!strcmp(attribute->Name(), "hist9_index4"))
                mSlpLtmPtr->hist9_index[4] = val;
            else if (!strcmp(attribute->Name(), "hist9_index5"))
                mSlpLtmPtr->hist9_index[5] = val;
            else if (!strcmp(attribute->Name(), "hist9_index6"))
                mSlpLtmPtr->hist9_index[6] = val;
            else if (!strcmp(attribute->Name(), "hist9_index7"))
                mSlpLtmPtr->hist9_index[7] = val;
            else if (!strcmp(attribute->Name(), "hist9_index8"))
                mSlpLtmPtr->hist9_index[8] = val;
            else if (!strcmp(attribute->Name(), "glb_x1"))
                mSlpLtmPtr->glb_x[0] = val;
            else if (!strcmp(attribute->Name(), "glb_x2"))
                mSlpLtmPtr->glb_x[1] = val;
            else if (!strcmp(attribute->Name(), "glb_x3"))
                mSlpLtmPtr->glb_x[2] = val;
            else if (!strcmp(attribute->Name(), "glb_s1"))
                mSlpLtmPtr->glb_s[0] = val;
            else if (!strcmp(attribute->Name(), "glb_s2"))
                mSlpLtmPtr->glb_s[1] = val;
            else if (!strcmp(attribute->Name(), "glb_s3"))
                mSlpLtmPtr->glb_s[2] = val;
            else if (!strcmp(attribute->Name(), "fast_ambient_th"))
                mSlpLtmPtr->fast_ambient_th = val;
            else if (!strcmp(attribute->Name(), "screen_change_percent_th"))
                mSlpLtmPtr->scene_change_percent_th = val;
            else if (!strcmp(attribute->Name(), "local_weight"))
                mSlpLtmPtr->local_weight = val;

            ALOGV("\t<param %s=%lu/>", attribute->Name(), val);
      }

      element = element->NextSiblingElement("param");
    }

    return 0;
}

/* Look at tags: <param xxx="xxx"/> */
int DpuR4P0SlpLtmData::parseAbcRegisterSceneValue(tinyxml2::XMLElement *node) {
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
            else if (!strcmp(attribute->Name(), "brightness_step"))
                mSlpLtmPtr->brightness_step = val;
            else if (!strcmp(attribute->Name(), "first_max_bright_th"))
                mSlpLtmPtr->first_max_bright_th = val;
            else if (!strcmp(attribute->Name(), "first_max_bright_th_step0"))
                mSlpLtmPtr->first_max_bright_th_step[0] = val;
            else if (!strcmp(attribute->Name(), "first_max_bright_th_step1"))
                mSlpLtmPtr->first_max_bright_th_step[1] = val;
            else if (!strcmp(attribute->Name(), "first_max_bright_th_step2"))
                mSlpLtmPtr->first_max_bright_th_step[2] = val;
            else if (!strcmp(attribute->Name(), "first_max_bright_th_step3"))
                mSlpLtmPtr->first_max_bright_th_step[3] = val;
            else if (!strcmp(attribute->Name(), "first_max_bright_th_step4"))
                mSlpLtmPtr->first_max_bright_th_step[4] = val;
            else if (!strcmp(attribute->Name(), "mask_height"))
                mSlpLtmPtr->mask_height = val;
            else if (!strcmp(attribute->Name(), "first_percent_th"))
                mSlpLtmPtr->fst_pth = val;
            else if (!strcmp(attribute->Name(), "first_pth_index0"))
                mSlpLtmPtr->first_pth_index[0] = val;
            else if (!strcmp(attribute->Name(), "first_pth_index1"))
                mSlpLtmPtr->first_pth_index[1] = val;
            else if (!strcmp(attribute->Name(), "first_pth_index2"))
                mSlpLtmPtr->first_pth_index[2] = val;
            else if (!strcmp(attribute->Name(), "first_pth_index3"))
                mSlpLtmPtr->first_pth_index[3] = val;
            else if (!strcmp(attribute->Name(), "slp_low_clip"))
                mSlpLtmPtr->limit_lclip = val;
            else if (!strcmp(attribute->Name(), "slp_high_clip"))
                mSlpLtmPtr->limit_hclip = val;
            else if (!strcmp(attribute->Name(), "slp_step_clip"))
                mSlpLtmPtr->limit_clip_step = val;

            ALOGV("\t<param %s=%lu/>", attribute->Name(), val);
        }

        element = element->NextSiblingElement("param");
    }

    return 0;
}
