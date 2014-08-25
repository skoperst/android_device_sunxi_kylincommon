#
# Copyright (C) 2013 The Android Open-Source Project
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

# cpu stuff
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_SMP := true
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_VARIANT := cortex-a15

TARGET_NO_BOOTLOADER := true
TARGET_PROVIDES_INIT_RC :=true

BOARD_USES_SECURE_SERVICES := true
TARGET_USE_NEON_OPTIMIZATION := true
TARGET_ARCH_LOWMEM := false
TARGET_BOARD_PLATFORM := kylin
TARGET_BOOTLOADER_BOARD_NAME := exdroid

BOARD_EGL_CFG := device/softwinner/kylin-common/egl/egl.cfg

USE_OPENGL_RENDERER := true
#VSYNC_EVENT_PHASE_OFFSET_NS := 7500000
#SF_VSYNC_EVENT_PHASE_OFFSET_NS := 5000000
TARGET_USES_ION := true

# audio & camera & cedarx
CEDARX_CHIP_VERSION := F39

BOARD_SEPOLICY_DIRS := \
    device/softwinner/kylin-common/sepolicy

# The list below is order dependent
BOARD_SEPOLICY_UNION := \
    app.te \
    device.te \
    domain.te \
    file_contexts \
    genfs_contexts \
    healthd.te \
    installd.te \
    mediaserver.te \
    mount.te \
    netd.te \
    surfaceflinger.te \
    system.te \
    untrusted_app.te \
    vold.te \
    zygote.te

# widevine
BOARD_WIDEVINE_OEMCRYPTO_LEVEL := 3
# hardware module include file path
TARGET_HARDWARE_INCLUDE := \
    device/softwinner/kylin-common/hardware/include
	
BOARD_CHARGER_ENABLE_SUSPEND := true

