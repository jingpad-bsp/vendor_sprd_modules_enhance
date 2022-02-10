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
#include "DpuEnhanceCore.h"
#include "EnhanceModule.h"
#include "DpuLiteR2P0XmlParser.h"
#include "DpuR4P0XmlParser.h"
#include <unistd.h>

#define ENHANCE_HEADER_VERSION 1

#define ENHANCE_DEVICE_API_VERSION_1_0 \
    HARDWARE_DEVICE_API_VERSION_2(1, 0, ENHANCE_HEADER_VERSION)

#define PQ_ID_FLASH "flash"

struct abc_context *abc;
struct bld_context *bld;
struct cms_context *cms;
struct gamma_context *gamma;
struct flash_context *flash;
struct dpu_cabc_context *cabc;

AbcXmlParser *abc_parser;
BldXmlParser *bld_parser;
CmsXmlParser *cms_parser;
GammaXmlParser *gamma_parser;
FlashXmlParser *flash_parser;

DpuEnhanceCore *dpu_enhance;

pthread_once_t g_init = PTHREAD_ONCE_INIT;
pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

void init_g_lock(void) {
    pthread_mutex_init(&g_lock, NULL);
}

int abc_set_mode(int mode) {
    int ret;

    pthread_mutex_lock(&g_lock);

    if (dpu_enhance)
        ret = dpu_enhance->abcSetMode(mode);
    else {
        ALOGE("%s dpu_enhance is NULL\n", __func__);
        ret = -1;
    }

    pthread_mutex_unlock(&g_lock);

    return ret;
}

int abc_set_value(int value) {
    int ret;

    pthread_mutex_lock(&g_lock);

    if (dpu_enhance)
        ret = dpu_enhance->slpSetBrightness(value);
    else {
        ALOGE("%s dpu_enhance is NULL\n", __func__);
        ret = -1;
    }

    pthread_mutex_unlock(&g_lock);

    return ret;
}

int cms_set_mode(int mode) {
    int ret;

    pthread_mutex_lock(&g_lock);

    if (dpu_enhance)
        ret = dpu_enhance->cmsSetMode(mode);
    else {
        ALOGE("%s dpu_enhance is NULL\n", __func__);
        ret = -1;
    }

    pthread_mutex_unlock(&g_lock);

    return ret;
}

int cms_set_value(int value) {
    int ret;

    pthread_mutex_lock(&g_lock);

    if (dpu_enhance)
        ret = dpu_enhance->cmsSetTemperature(value);
    else {
        ALOGE("%s dpu_enhance is NULL\n", __func__);
        ret = -1;
    }

    pthread_mutex_unlock(&g_lock);

    return ret;
}

int bld_set_mode(int mode) {
    int ret;

    pthread_mutex_lock(&g_lock);

    if (dpu_enhance)
        ret = dpu_enhance->bldSetMode(mode);
    else {
        ALOGE("%s dpu_enhance is NULL\n", __func__);
        ret = -1;
    }

    pthread_mutex_unlock(&g_lock);

    return ret;
}

int gamma_set_mode(int mode) {
    int ret;

    pthread_mutex_lock(&g_lock);

    if (dpu_enhance)
        ret = dpu_enhance->gammaSetMode(mode);
    else {
        ALOGE("%s dpu_enhance is NULL\n", __func__);
        ret = -1;
    }

    pthread_mutex_unlock(&g_lock);

    return ret;
}

int cabc_set_mode(int mode) {
    int ret;

    pthread_mutex_lock(&g_lock);

    if (dpu_enhance)
        ret = dpu_enhance->cabcSetMode(mode);
    else {
        ALOGE("%s dpu_enhance is NULL\n", __func__);
        ret = -1;
    }

    pthread_mutex_unlock(&g_lock);

    return ret;
}

int flash_set_value(int value) {
    int ret;

    pthread_mutex_lock(&g_lock);

    if (dpu_enhance)
        ret = dpu_enhance->flashSetTemperature(value);
    else {
        ALOGE("%s dpu_enhance is NULL\n", __func__);
        ret = -1;
    }

    pthread_mutex_unlock(&g_lock);

    return ret;
}

int get_dpu_version(char *version, int count) {
    int fd;
    int ret;

    fd = open(FILE_DPU_VERSION, O_RDONLY);
    if (fd >= 0) {
        ret = read(fd, version, count);
        if (ret > 0) {
            version[ret - 1] = '\0';
            ALOGD("dpu version:%s", version);
        } else
            ALOGE("%s get dpu version failed ret:%d\n", __func__, ret);
        close(fd);
    } else {
        ALOGE("%s: open %s failed errno:%d\n", __func__, FILE_DPU_VERSION, errno);
        ret = -1;
    }

    return (ret < 0) ? -errno : ret;
}

int get_panel_name(char *name, int count) {
    int fd;
    int ret;

    fd = open(FILE_PANEL_NAME, O_RDONLY);
    if (fd >= 0) {
        ret = read(fd, name, count);
        if (ret > 0) {
            name[ret - 1] = '\0';
            ALOGD("%s:%s", __func__, name);
        } else
            ALOGE("%s fail:%d", __func__, errno);
        close(fd);
    } else {
        ALOGE("%s: open %s fail:%d", __func__, FILE_PANEL_NAME, errno);
        ret = -1;
    }

    return (ret < 0) ? -errno : ret;
}

int get_gamma_xml_file_name(char *prod_file, int prod_file_len,
                char *vendor_file, int vendor_file_len) {
    char name[256];
    char suffix[] = "_gamma.xml";
    int len;
    int ret;

    len = strlen(vendor_file) + 1;

    ret = get_panel_name(name, (sizeof(name) - strlen(suffix)));
    if (ret > 0) {
        strcat(name, suffix);
        strncat(vendor_file, name, (vendor_file_len - len));

        ret = access(vendor_file, F_OK);
        if (!ret) {
            len = strlen(prod_file) + 1;
            strncat(prod_file, name, (prod_file_len - len));
            return 0;
        } else {
            ALOGD("file %s doesn't exist, use default gamma xml", vendor_file);
            vendor_file[len - 1] = '\0';
        }
    }

    /* use default gamma xml */
    strncat(vendor_file, "gamma.xml", (vendor_file_len - len));
    len = strlen(prod_file) + 1;
    strncat(prod_file, "gamma.xml", (prod_file_len - len));

    return 0;
}

int check_xml_file_version(const char *prod_file, const char *vendor_file)
{
    VersionXmlParser *version_parser;
    int ret;

    version_parser = new VersionXmlParser();

    ret = version_parser->enhanceFileCheck(prod_file, vendor_file);
    delete version_parser;

    return ret;
}

void create_dpu_xml_parser(const char *name) {
    if (!strcmp(PQ_ID_SLP, name) && abc)
        abc_parser = new AbcXmlParser();
    else if (!strcmp(PQ_ID_BLP, name) && bld)
        bld_parser = new BldXmlParser();
    else if (!strcmp(PQ_ID_CMS, name) && cms)
        cms_parser = new CmsXmlParser();
    else if (!strcmp(PQ_ID_GAM, name) && gamma)
        gamma_parser = new GammaXmlParser();
    else if (!strcmp(PQ_ID_FLASH, name) && flash)
        flash_parser = new FlashXmlParser();
}

void create_dpu_lite_r2p0_xml_parser(const char *name) {
    if (!strcmp(PQ_ID_SLP, name) && abc)
        abc_parser = new DpuLiteR2P0AbcXmlParser();
    else if (!strcmp(PQ_ID_BLP, name) && bld)
        bld_parser = new DpuLiteR2P0BldXmlParser();
    else if (!strcmp(PQ_ID_CMS, name) && cms)
        cms_parser = new DpuLiteR2P0CmsXmlParser();
    else if (!strcmp(PQ_ID_GAM, name) && gamma)
        gamma_parser = new GammaXmlParser();
    else if (!strcmp(PQ_ID_FLASH, name) && flash)
        flash_parser = new FlashXmlParser();
}

void create_dpu_r4p0_xml_parser(const char *name) {
    if (!strcmp(PQ_ID_SLP, name) && abc)
        abc_parser = new DpuR4P0AbcXmlParser();
    else if (!strcmp(PQ_ID_BLP, name) && bld)
        bld_parser = new DpuR4P0BldXmlParser();
    else if (!strcmp(PQ_ID_CMS, name) && cms)
        cms_parser = new DpuR4P0CmsXmlParser();
    else if (!strcmp(PQ_ID_GAM, name) && gamma)
        gamma_parser = new GammaXmlParser();
    else if (!strcmp(PQ_ID_FLASH, name) && flash)
        flash_parser = new FlashXmlParser();
}

int parse_xml_file(const char *version,
        const char *id, const char *path) {
    int ret = -1;

    if (!strcmp("dpu-r2p0", version))
        create_dpu_xml_parser(id);
    else if (!strcmp("dpu-lite-r2p0", version) ||
        !strcmp("dpu-r3p0", version))
        create_dpu_lite_r2p0_xml_parser(id);
    else if (!strcmp("dpu-r4p0", version))
        create_dpu_r4p0_xml_parser(id);
    else
        ALOGE("%s: unknown dpu version", __func__);

    if (!strcmp(PQ_ID_SLP, id) && abc_parser) {
        ret = abc_parser->parseXmlData(path);
        delete abc_parser;
        abc_parser = NULL;
    } else if (!strcmp(PQ_ID_BLP, id) && bld_parser) {
        ret = bld_parser->parseXmlData(path);
        delete bld_parser;
        bld_parser = NULL;
    } else if (!strcmp(PQ_ID_CMS, id) && cms_parser) {
        ret = cms_parser->parseXmlData(path);
        delete cms_parser;
        cms_parser = NULL;
    } else if (!strcmp(PQ_ID_GAM, id) && gamma_parser) {
        ret = gamma_parser->parseXmlData(path);
        delete gamma_parser;
        gamma_parser = NULL;
        if (ret)
            gamma->num_modes = 0;
    } else if (!strcmp(PQ_ID_FLASH, id) && flash_parser) {
        ret = flash_parser->parseXmlData(path);
        delete flash_parser;
        flash_parser = NULL;
    }

    if (ret)
        ALOGE("%s: parse %s failed\n", __func__, id);

    return ret;
}

int get_enhance_param_from_xml(const char *id, const char *version) {
    char prod_file[MAX_XML_PATH_LENGTH] = {};
    char vendor_file[MAX_XML_PATH_LENGTH] = {};
    const char *xml_file[] = {
        PQ_ID_SLP, "abc.xml",
        PQ_ID_BLP, "bld.xml",
        PQ_ID_CMS, "cms.xml",
        PQ_ID_CMS, "3d_lut.xml",
        PQ_ID_GAM, "gamma.xml",
        PQ_ID_FLASH, "flash.xml"
    };
    int i, num;
    int ret;
    int file_num = 0;

    if (!strcmp(id, PQ_ID_CABC))
        return 0;

    num = sizeof(xml_file) / sizeof(xml_file[0]);
    for (i = 0; i < num; i += 2) {
        if (!strcmp(id, xml_file[i])) {
            strncpy(vendor_file, VENDOR_FOLDER, sizeof(VENDOR_FOLDER));
            strncpy(prod_file, PROD_FOLDER, sizeof(PROD_FOLDER));

            if (!strcmp(id, PQ_ID_GAM)) {
                get_gamma_xml_file_name(prod_file, sizeof(prod_file),
                        vendor_file, sizeof(vendor_file));
            } else {
                strncat(prod_file, xml_file[i + 1],
                    (sizeof(prod_file) - sizeof(PROD_FOLDER)));
                strncat(vendor_file, xml_file[i + 1],
                    (sizeof(vendor_file) - sizeof(VENDOR_FOLDER)));
            }

            ret = check_xml_file_version(prod_file, vendor_file);
            if (ret)
                break;

            ret = parse_xml_file(version, id, prod_file);
            if (ret) {
                ALOGE("%s: parse xml fail", __func__);
                break;
            }

            file_num++;
        }
    }

    return file_num ? 0 : -1;
}

int create_dpu_enhance_core(const char *version) {
    if (!strcmp("dpu-r2p0", version))
        dpu_enhance = new DpuR2P0EnhanceCore();
    else if (!strcmp("dpu-lite-r2p0", version))
        dpu_enhance = new DpuLiteR2P0EnhanceCore();
    else if (!strcmp("dpu-r3p0", version))
         dpu_enhance = new DpuR3P0EnhanceCore();
    else if (!strcmp("dpu-r4p0", version))
        dpu_enhance = new DpuR4P0EnhanceCore();
    else {
        ALOGE("%s: unknown dpu version", __func__);
        return -1;
    }

    return 0;
}

int is_pq_disabled(const char *name) {
    char value[PROPERTY_VALUE_MAX];
    unsigned long int disable;

    property_get("persist.vendor.pq.disable", value, "0");
    disable = strtoul(value, NULL, 0);

    if ((!strcmp(PQ_ID_SLP, name) && (disable & PQ_ABC)) ||
        (!strcmp(PQ_ID_CMS, name) && (disable & PQ_CMS)) ||
        (!strcmp(PQ_ID_BLP, name) && (disable & PQ_BLP)) ||
        (!strcmp(PQ_ID_GAM, name) && (disable & PQ_GAM)) ||
        (!strcmp(PQ_ID_FLASH, name) && (disable & PQ_FLASH))) {
        ALOGD("%s is disabled:0x%lx", name, disable);
        return 1;
    }

    return 0;
}

static int enhance_device_close(struct hw_device_t *dev) {

    pthread_mutex_lock(&g_lock);

    ALOGI("%s: close enhance device", __func__);

    if (dpu_enhance) {
        dpu_enhance->mCount--;
        if (dpu_enhance->mCount == 0) {
            delete dpu_enhance;
            dpu_enhance = NULL;
        }
    }

    free(dev);

    pthread_mutex_unlock(&g_lock);

    return 0;
}

static int enhance_device_open(const hw_module_t *module,
            const char *name, hw_device_t **device) {

    ALOGI("%s: open %s device", __func__, name);

    int ret;
    static char dpu_version[20];
    enhance_device_t *dev;

    if (is_pq_disabled(name))
        return -EINVAL;

    if (strlen(dpu_version) == 0) {
        ret = get_dpu_version(dpu_version, sizeof(dpu_version));
        if (ret <= 0)
            return -EINVAL;
    }

    if (!dpu_enhance) {
        ret = create_dpu_enhance_core(dpu_version);
        if (ret)
            return -EINVAL;
    }

    dpu_enhance->mCount++;

    ret = get_enhance_param_from_xml(name, dpu_version);
    if (ret) {
        ALOGE("%s: get enhance param fail", __func__);
        goto err;
    }

    dev = (enhance_device_t *)malloc(sizeof(enhance_device_t));
    memset(dev, 0, sizeof(*dev));

    if (!strcmp(PQ_ID_SLP, name)) {
        dev->set_value = abc_set_value;
        dev->set_mode = abc_set_mode;
    } else if (!strcmp(PQ_ID_CMS, name)) {
        dev->set_value = cms_set_value;
        dev->set_mode = cms_set_mode;
    } else if (!strcmp(PQ_ID_BLP, name)) {
        dev->set_mode = bld_set_mode;
    } else if (!strcmp(PQ_ID_GAM, name)) {
        dev->set_mode = gamma_set_mode;
        if (gamma->num_modes == 1)
            gamma_set_mode(GAMMA_MODE_DEFAULT);
    } else if (!strcmp(PQ_ID_FLASH, name)) {
        dev->set_value = flash_set_value;
    } else if (!strcmp(PQ_ID_CABC, name)) {
        dev->set_mode = cabc_set_mode;
    } else {
        free(dev);
        goto err;
    }

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = ENHANCE_DEVICE_API_VERSION_1_0;
    dev->common.module = (hw_module_t *)module;
    dev->common.close = enhance_device_close;

    *device = (hw_device_t *)dev;

    pthread_once(&g_init, init_g_lock);

    return 0;

err:
    if (--dpu_enhance->mCount == 0) {
        ALOGD("delete dpu_enhance\n");
        delete dpu_enhance;
        dpu_enhance = NULL;
    }

    return -EINVAL;
}

static hw_module_methods_t enhance_module_methods = {
    .open = enhance_device_open
};

hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = ENHANCE_HARDWARE_MODULE_ID,
    .name = "Picture Quality Enhance Module",
    .author = "Spreadtrum Communications, Inc",
    .methods = &enhance_module_methods,
    .dso = 0,
    .reserved = {0},
};
