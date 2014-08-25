#
# Copyright (C) 2008 Google Inc.
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

###############################################################################
# GoogleCamera
#include $(CLEAR_VARS)
#LOCAL_PREBUILT_LIBS := libgcam.so \
#                       libgcam_swig_jni.so \
#                       libjni_mosaic.so \
#                       libjni_tinyplanet.so \
#                       libjpeg.so \
#                       liblightcycle.so \
#                       libnativehelper_compat.so
#LOCAL_MODULE_TAGS := optional
#include $(BUILD_MULTI_PREBUILT)

###############################################################################
# GalleryGoogle
#include $(CLEAR_VARS)
#LOCAL_PREBUILT_LIBS := libjni_eglfence.so \
#                       libjni_filtershow_filters.so \
#                       librsjni.so
#LOCAL_MODULE_TAGS := optional
#include $(BUILD_MULTI_PREBUILT)

###############################################################################
# Facelock
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libfacelock_jni.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)

###############################################################################
# Media Effects library
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libfilterpack_facedetect.so \
                       libfrsdk.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)

###############################################################################
# WebView Chromium
#include $(CLEAR_VARS)
#LOCAL_PREBUILT_LIBS := libwebviewchromium.so
#LOCAL_MODULE_TAGS := optional
#include $(BUILD_MULTI_PREBUILT)

###############################################################################
# Chrome
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libchrome.1916.122.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)

###############################################################################
# Drive
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libdocscanner_image-v7a.so \
                       libdocsimageutils.so \
                       liblinearalloc.so \
                       libndk1.so \
                       librectifier-v7a.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)

###############################################################################
# PlusOne
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libcrashreporter.so \
                       libfilterframework_jni.so \
                       libmoviemaker-jni.so \
                       libnetjni.so \
                       libphotoeditor_native.so \
                       libwebp_android.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)

###############################################################################
# GmsCore
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libAppDataSearch.so \
                       libgames_rtmp_jni.so \
                       libgcastv2_base.so \
                       libgcastv2_support.so \
                       libjgcastservice.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)

###############################################################################
# Velvet
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libgoogle_hotword_jni.so \
                       libgoogle_recognizer_jni_l.so \
                       libvcdecoder_jni.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)

###############################################################################
# Hangouts
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libvideochat_jni.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)

###############################################################################
# Videos
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libWVphoneAPI.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)

###############################################################################
# GoogleTTS
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libpatts_engine_jni_api_ub.210030011.so \
                       libspeexwrapper.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)

###############################################################################
# LatinIMEGoogle
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libjni_unbundled_latinimegoogle.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)

