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

#define BUFFER_SIZE 1024

int VersionXmlParser::parseVersion(const char *path, char (*version)[20]) {
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLAttribute* attribute = NULL;
    tinyxml2::XMLError err;
    tinyxml2::XMLElement *root;
    tinyxml2::XMLElement *element = NULL;

    err = doc.LoadFile(path);
    if (tinyxml2::XML_SUCCESS == err) {
        root = doc.RootElement();
        if (root)
            element = root->FirstChildElement("enhance");

        while (element) {
            attribute = element->FirstAttribute();
            if (attribute && !strcmp(attribute->Name(), "major_version")) {
                /* Look at tags: <enhance major_version="xxxxxx"/> */
                strcpy(version[0], attribute->Value());
                ALOGV("parseVersion <enhance major_version=%s/>\n", attribute->Value());
            } else if (attribute && !strcmp(attribute->Name(), "version")) {
                /* Look at tags: <enhance version="xxxxxx" /> */
                strcpy(version[1], attribute->Value());
                ALOGV("parseVersion <enhance version=%s/>\n", attribute->Value());
                return 0;
            }

            element = element->NextSiblingElement("enhance");
        }
    } else
        ALOGE("parseVersion LoadFile file:%s failed:%d\n", path, err);

    ALOGE("parseVersion no version\n");
    return -1;
}

int VersionXmlParser::copy(const char *dst, const char *src)
{
    FILE *fin, *fout;
    char *buf;
    int ret, bytes;

    fin = fopen(src, "r");
    if (NULL == fin) {
        ALOGE("open file %s failed", src);
        return errno;
    }

    fout = fopen(dst,"w");
    if (NULL == fout) {
        ALOGE("open file %s failed", dst);
        ret = errno;
        goto err0;
    }

    buf = (char *)malloc(BUFFER_SIZE);
    if (NULL == buf) {
        ALOGE("malloc failed");
        ret = -ENOMEM;
        goto err1;
    }

    while (1) {
        bytes = fread(buf, 1, BUFFER_SIZE, fin);

        if (bytes != BUFFER_SIZE)
            fwrite(buf, bytes, 1, fout);
        else
            fwrite(buf, BUFFER_SIZE, 1, fout);

        if (feof(fin))
            break;
    }

    if (chmod(dst, 0660))
        ALOGE("chmod %s failed:%d\n", dst, errno);

    fclose(fin);
    fclose(fout);
    free(buf);

    return 0;

err1:
    fclose(fout);
err0:
    fclose(fin);

    return ret;
}

int VersionXmlParser::enhanceFileCheck(const char *prod_file, const char *vendor_file)
{
    char prod_ver[2][20] = {};
    char vendor_ver[2][20] = {};
    const char *prod_dir = "/mnt/vendor/enhance";

    if (access(vendor_file, F_OK)) {
        ALOGE("file %s doesn't exist", vendor_file);
        return errno;
    }

    if (access(prod_file, F_OK)) {

        if (access(prod_dir, F_OK)) {
            ALOGE("%s directory doesn't exist, create it", prod_dir);
            if (mkdir(prod_dir, 755)) {
                ALOGE("create %s directory failed", prod_dir);
                return errno;
            }
            ALOGI("start copy %s from /vendor", prod_file);
        } else
            ALOGE("file %s doesn't exist, start copy from /vendor", prod_file);

        if (copy(prod_file, vendor_file)) {
            ALOGE("copy file %s failed", prod_file);
            return -ENOENT;
        }
        return 0;
    }

    if (parseVersion(prod_file, prod_ver)) {
        ALOGE("file %s has no version, start copy from /vendor", prod_file);
        if (copy(prod_file, vendor_file)) {
            ALOGE("copy file %s failed", prod_file);
            return -ENOENT;
        }
        return 0;
    }

    if (parseVersion(vendor_file, vendor_ver)) {
        ALOGE("file %s has no version, stop copy from /vendor", prod_file);
        return 0;
    }

    /*compare enhance major_version*/
    if (strcmp(prod_ver[0], vendor_ver[0])) {
        ALOGI("major_version:file %s needs to be updated, start copy from /vendor", prod_file);
        if (copy(prod_file, vendor_file)) {
            ALOGE("copy file %s failed", prod_file);
            return -ENOENT;
        }
        return 0;
    }

    if (strcmp(prod_ver[1], vendor_ver[1]) < 0) {
        ALOGI("file %s needs to be updated, start copy from /vendor", prod_file);
        if (copy(prod_file, vendor_file)) {
            ALOGE("copy file %s failed", prod_file);
            return -ENOENT;
        }
        return 0;
    }

    if (chmod(prod_file, 0660))
        ALOGE("chmod %s failed:%d\n", prod_file, errno);

    return 0;
}
