
# Copyright (C) 2008 The Android Open Source Project
# Copyright (C) 2012 Broadcom Corporation
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
#

LOCAL_PATH := $(call my-dir)

################################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := 4KPlayer
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
include $(BUILD_PREBUILT)

################################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := AWGallery
LOCAL_MODULE_TAGS := optional
#LOCAL_OVERRIDES_PACKAGES := Camera2
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_REQUIRED_MODULES := libjni_eglfence_awgallery \
                          libjni_mosaic
include $(BUILD_PREBUILT)

################################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := GooglePinyin
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := PinyinIME
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_REQUIRED_MODULES := libgnustl_shared \
                          libhwr \
                          libhwrword \
                          libhwrzhmodel \
                          libjni_delight \
                          libjni_googlepinyinime_latinime_5 \
                          libjni_hmm_shared_engine \
                          libpinyin_data_bundle
include $(BUILD_PREBUILT)
