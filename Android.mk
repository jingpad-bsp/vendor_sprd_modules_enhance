# Copyright (C) 2017 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

ifeq ($(strip $(SPRD_CABC)), true)
include $(CLEAR_VARS)

LOCAL_MODULE := libomx_cabc_sw_sprd
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MULTILIB := both
LOCAL_MODULE_STEM_32 := $(LOCAL_MODULE).so
LOCAL_MODULE_STEM_64 := $(LOCAL_MODULE).so
LOCAL_SRC_FILES_32 := libs/arm/libomx_cabc_sw_sprd.so
LOCAL_SRC_FILES_64 := libs/arm64/libomx_cabc_sw_sprd.so

include $(BUILD_PREBUILT)

######################################################################
endif
include $(CLEAR_VARS)

ifeq ($(strip $(SPRD_CABC)), true)
LOCAL_CFLAGS += -DSPRD_CABC
endif

LOCAL_C_INCLUDES += \
		$(LOCAL_PATH)/XmlParser/ \
		$(LOCAL_PATH)/XmlParser/DpuLiteR2P0/ \
		$(LOCAL_PATH)/XmlParser/DpuR4P0/ \
		$(LOCAL_PATH)/Core/

ifeq ($(strip $(SPRD_CABC)), true)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc/
endif

LOCAL_SRC_FILES := EnhanceModule.cpp \
		   Core/DpuEnhanceCore.cpp \
		   XmlParser/XmlData.cpp \
		   XmlParser/VersionXmlParser.cpp \
		   XmlParser/AbcXmlParser.cpp \
		   XmlParser/BldXmlParser.cpp \
		   XmlParser/CmsXmlParser.cpp \
		   XmlParser/GammaXmlParser.cpp \
		   XmlParser/FlashXmlParser.cpp

LOCAL_SRC_FILES += Core/DpuR2P0EnhanceCore.cpp

LOCAL_SRC_FILES += Core/DpuLiteR2P0EnhanceCore.cpp \
		XmlParser/DpuLiteR2P0/DpuLiteR2P0BldXmlParser.cpp \
		XmlParser/DpuLiteR2P0/DpuLiteR2P0AbcXmlParser.cpp \
		XmlParser/DpuLiteR2P0/DpuLiteR2P0CmsXmlParser.cpp \
		Core/DpuR4P0EnhanceCore.cpp \
		XmlParser/DpuR4P0/DpuR4P0CmsXmlParser.cpp \
		XmlParser/DpuR4P0/DpuR4P0AbcXmlParser.cpp

ifeq ($(strip $(SPRD_CABC)), true)
LOCAL_SRC_FILES += Cabc.cpp \
                Core/DpuR3P0EnhanceCore.cpp
endif

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_SHARED_LIBRARIES := liblog libtinyxml2 libhardware libcutils

ifeq ($(strip $(SPRD_CABC)), true)
LOCAL_SHARED_LIBRARIES += libomx_cabc_sw_sprd
endif

LOCAL_MODULE := enhance.$(TARGET_BOARD_PLATFORM)

LOCAL_MODULE_TAGS := optional

ifeq ($(strip $(SPRD_CABC)), true)
# Install the symlinks.
LOCAL_POST_INSTALL_CMD = ln -sf ./hw/$(LOCAL_MODULE).so $(dir $(LOCAL_INSTALLED_MODULE))/../
endif

include $(BUILD_SHARED_LIBRARY)


######################################################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES := Test.cpp \
		   EnhanceModule.cpp \
		   Core/DpuEnhanceCore.cpp \
		   XmlParser/XmlData.cpp \
		   XmlParser/VersionXmlParser.cpp \
		   XmlParser/AbcXmlParser.cpp \
		   XmlParser/BldXmlParser.cpp \
		   XmlParser/CmsXmlParser.cpp \
		   XmlParser/GammaXmlParser.cpp \
		   XmlParser/FlashXmlParser.cpp

LOCAL_SRC_FILES += Core/DpuR2P0EnhanceCore.cpp

LOCAL_SRC_FILES += Core/DpuLiteR2P0EnhanceCore.cpp \
		XmlParser/DpuLiteR2P0/DpuLiteR2P0BldXmlParser.cpp \
		XmlParser/DpuLiteR2P0/DpuLiteR2P0AbcXmlParser.cpp \
		XmlParser/DpuLiteR2P0/DpuLiteR2P0CmsXmlParser.cpp \
		Core/DpuR4P0EnhanceCore.cpp \
		XmlParser/DpuR4P0/DpuR4P0CmsXmlParser.cpp \
		XmlParser/DpuR4P0/DpuR4P0AbcXmlParser.cpp

ifeq ($(strip $(SPRD_CABC)), true)
LOCAL_SRC_FILES += Cabc.cpp \
                   Core/DpuR3P0EnhanceCore.cpp
endif

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/XmlParser \
    $(LOCAL_PATH)/XmlParser/DpuLiteR2P0 \
    $(LOCAL_PATH)/XmlParser/DpuR4P0 \
    $(LOCAL_PATH)/Core

ifeq ($(strip $(SPRD_CABC)), true)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc/
endif

LOCAL_SHARED_LIBRARIES := \
    libhardware_legacy \
    libtinyxml2 \
    libc \
    libcutils \
    liblog \
    libutils \
    libhardware

ifeq ($(strip $(SPRD_CABC)), true)
LOCAL_SHARED_LIBRARIES += libomx_cabc_sw_sprd
endif

LOCAL_MODULE := enhance_test
#LOCAL_INIT_RC := test.rc
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)


######################################################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES := FlashTest.cpp

LOCAL_PROPRIETARY_MODULE := true

LOCAL_SHARED_LIBRARIES := libhardware libcutils

LOCAL_MODULE := flash_test

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)


######################################################################
# There is no need to use enhance_check.sh for xml version check.
#
#include $(CLEAR_VARS)
#
#LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE := enhance_check.sh
#LOCAL_MODULE_CLASS := EXECUTABLES
#LOCAL_SRC_FILES := enhance_check.sh
#LOCAL_INIT_RC := enhance_check.rc
#LOCAL_PROPRIETARY_MODULE := true
#
#include $(BUILD_PREBUILT)

