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
# GMS Mandatory Apps (not published in Play Store)
# This binary is required for any Google application to work.
# It MUST be installed on all devices.
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := GoogleBackupTransport
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := ConfigUpdater
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := GoogleFeedback
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := GoogleServicesFramework
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := GoogleLoginService
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := GooglePartnerSetup
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := SetupWizard
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := Provision
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := GoogleContactsSyncAdapter
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := GoogleCalendarSyncAdapter
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := Phonesky
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
# Chrome browser
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := Chrome
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := Browser
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_REQUIRED_MODULES := libchrome.1916.122 \
                          BrowserProviderProxy \
                          PartnerBookmarksProvider
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := ChromeWithBrowser
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := Chrome.apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_REQUIRED_MODULES := libchrome.1847.114 \
                          PartnerBookmarksProvider
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := BrowserProviderProxy
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := platform
include $(BUILD_PREBUILT)

###############################################################################
# Install PartnerHomepageProviderExample.apk as /vendor/app/ChromeCustomizations.apk
# This is for an example. Do not distribute this apk in your production devices
# Note 1: this example sets "www.google.com" as the default homepage of Chrome
# Note 2: ChromeCustomizations.apk should be installed in /vendor/app, however,
# if /vendor is symbolic linked to /system/vendor, it's subject to error.
# In this case, apply this AOSP patch: https://android-review.googlesource.com/#/c/80435/
include $(CLEAR_VARS)
LOCAL_MODULE := ChromeCustomizations
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/app
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := PartnerHomepageProviderExample.apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
# GMS Mandatory Apps (published in Play Store)
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := Books
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := Drive
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_REQUIRED_MODULES := libdocscanner_image-v7a \
                          libdocsimageutils \
                          liblinearalloc \
                          libndk1 \
                          librectifier-v7a
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := Magazines
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := Maps
LOCAL_MODULE_TAGS := optional
prebuilt_maps_PRODUCT_AAPT_CONFIG := $(subst $(comma), ,$(PRODUCT_AAPT_CONFIG))
ifneq (,$(filter xxhdpi,$(prebuilt_maps_PRODUCT_AAPT_CONFIG)))
LOCAL_SRC_FILES := Maps_xxhdpi.apk
else ifneq (,$(filter xhdpi,$(prebuilt_maps_PRODUCT_AAPT_CONFIG)))
LOCAL_SRC_FILES := Maps_xhdpi.apk
else ifneq (,$(filter hdpi,$(prebuilt_maps_PRODUCT_AAPT_CONFIG)))
LOCAL_SRC_FILES := Maps_hdpi.apk
else ifneq (,$(filter mdpi,$(prebuilt_maps_PRODUCT_AAPT_CONFIG)))
LOCAL_SRC_FILES := Maps_mdpi.apk
else
LOCAL_SRC_FILES := Maps_alldpi.apk
endif
prebuilt_maps_PRODUCT_AAPT_CONFIG :=
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_REQUIRED_MODULES := com.google.android.maps.jar
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := PlusOne
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_REQUIRED_MODULES := libcrashreporter \
                          libfilterframework_jni \
                          libmoviemaker-jni \
                          libnetjni \
                          libphotoeditor_native \
                          libwebp_android
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := Gmail2
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := GmsCore
LOCAL_MODULE_TAGS := optional
prebuilt_gmscore_PRODUCT_AAPT_CONFIG := $(subst $(comma), ,$(PRODUCT_AAPT_CONFIG))
ifneq (,$(filter xxhdpi,$(prebuilt_gmscore_PRODUCT_AAPT_CONFIG)))
LOCAL_SRC_FILES := GmsCore_xxhdpi.apk
else ifneq (,$(filter xhdpi,$(prebuilt_gmscore_PRODUCT_AAPT_CONFIG)))
LOCAL_SRC_FILES := GmsCore_xhdpi.apk
else ifneq (,$(filter hdpi,$(prebuilt_gmscore_PRODUCT_AAPT_CONFIG)))
LOCAL_SRC_FILES := GmsCore_hdpi.apk
else ifneq (,$(filter mdpi,$(prebuilt_gmscore_PRODUCT_AAPT_CONFIG)))
LOCAL_SRC_FILES := GmsCore_mdpi.apk
else
LOCAL_SRC_FILES := GmsCore_alldpi.apk
endif
prebuilt_gmscore_PRODUCT_AAPT_CONFIG :=
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_REQUIRED_MODULES := libAppDataSearch \
                          libgames_rtmp_jni \
                          libgcastv2_base \
                          libgcastv2_support \
                          libjgcastservice
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := Velvet
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := QuickSearchBox
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_REQUIRED_MODULES := \
    libgoogle_hotword_jni \
    libgoogle_recognizer_jni_l \
    libvcdecoder_jni \
    en-US/c_fst \
    en-US/clg \
    en-US/commands.abnf \
    en-US/compile_grammar.config \
    en-US/contacts.abnf \
    en-US/dict \
    en-US/dictation.config \
    en-US/dnn \
    en-US/endpointer_dictation.config \
    en-US/endpointer_voicesearch.config \
    en-US/ep_acoustic_model \
    en-US/g2p_fst \
    en-US/grammar.config \
    en-US/hclg_shotword \
    en-US/hmmlist \
    en-US/hmm_symbols \
    en-US/hotword_classifier \
    en-US/hotword.config \
    en-US/hotword_normalizer \
    en-US/hotword_prompt.txt \
    en-US/hotword_word_symbols \
    en-US/metadata \
    en-US/normalizer \
    en-US/norm_fst \
    en-US/offensive_word_normalizer \
    en-US/phonelist \
    en-US/phone_state_map \
    en-US/rescoring_lm \
    en-US/wordlist
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := Music2
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := Music
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := PlayGames
LOCAL_MODULE_TAGS := optional
prebuilt_playgames_PRODUCT_AAPT_CONFIG := $(subst $(comma), ,$(PRODUCT_AAPT_CONFIG))
ifneq (,$(filter xxhdpi,$(prebuilt_playgames_PRODUCT_AAPT_CONFIG)))
LOCAL_SRC_FILES := PlayGames_xxhdpi.apk
else ifneq (,$(filter xhdpi,$(prebuilt_playgames_PRODUCT_AAPT_CONFIG)))
LOCAL_SRC_FILES := PlayGames_xhdpi.apk
else ifneq (,$(filter hdpi,$(prebuilt_playgames_PRODUCT_AAPT_CONFIG)))
LOCAL_SRC_FILES := PlayGames_hdpi.apk
else ifneq (,$(filter mdpi,$(prebuilt_playgames_PRODUCT_AAPT_CONFIG)))
LOCAL_SRC_FILES := PlayGames_mdpi.apk
else
LOCAL_SRC_FILES := PlayGames_alldpi.apk
endif
prebuilt_playgames_PRODUCT_AAPT_CONFIG :=
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := Street
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := Hangouts
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := Talk Mms
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_REQUIRED_MODULES := libvideochat_jni
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := Videos
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_REQUIRED_MODULES := libWVphoneAPI
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := YouTube
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
# GMS Optional Apps (not published in Play Store)
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := GoogleCamera
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := Camera2
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_REQUIRED_MODULES := libgcam \
                          libgcam_swig_jni \
                          libjni_mosaic \
                          libjni_tinyplanet \
                          libjpeg \
                          liblightcycle \
                          libnativehelper_compat
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := DeskClockGoogle
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := DeskClock
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := FaceLock
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_REQUIRED_MODULES := libfacelock_jni \
    detection/multi_pose_face_landmark_detectors.7/left_eye-y0-yi45-p0-pi45-r0-ri20.lg_32.bin \
    detection/multi_pose_face_landmark_detectors.7/nose_base-y0-yi45-p0-pi45-r0-ri20.lg_32.bin \
    detection/multi_pose_face_landmark_detectors.7/right_eye-y0-yi45-p0-pi45-r0-ri20.lg_32-2.bin \
    detection/yaw_roll_face_detectors.6/head-y0-yi45-p0-pi45-r0-ri30.4a-v24.bin \
    detection/yaw_roll_face_detectors.6/head-y0-yi45-p0-pi45-rn30-ri30.5-v24.bin \
    detection/yaw_roll_face_detectors.6/head-y0-yi45-p0-pi45-rp30-ri30.5-v24.bin \
    recognition/face.face.y0-y0-22-b-N.bin
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := GalleryGoogle
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := Gallery2
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_REQUIRED_MODULES := libjni_eglfence \
                          libjni_filtershow_filters \
                          libjpeg \
                          librsjni
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := MediaUploader
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := GoogleOneTimeInitializer
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := VideoEditorGoogle
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := VideoEditor
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := TagGoogle
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := Tag
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
# GMS Optional Apps (published in Play Store)
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := CalendarGoogle
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := Calendar GoogleCalendarSyncAdapter
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := CloudPrint
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := GoogleTTS
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := PicoTts
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_REQUIRED_MODULES := libpatts_engine_jni_api_ub.210030011 \
                          libspeexwrapper
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := LatinImeGoogle
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := LatinIME
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_REQUIRED_MODULES := libjni_unbundled_latinimegoogle
include $(BUILD_PREBUILT)

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := talkback
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

