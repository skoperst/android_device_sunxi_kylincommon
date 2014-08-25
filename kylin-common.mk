$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

DEVICE_PACKAGE_OVERLAYS := \
    device/softwinner/kylin-common/overlay
	
include device/softwinner/kylin-common/prebuild/tools/tools.mk
PRODUCT_PACKAGES += \
    libion \
    sensors.kylin \
    lights.kylin \
    keystore.kylin \
	hwcomposer.kylin

PRODUCT_PACKAGES += SoundRecorder

#app
PRODUCT_PACKAGES += \
	FileExplore \
	Update

#charge
PRODUCT_PACKAGES +=\
	charger_res_images \
	charger
	
#system app
PRODUCT_PACKAGES += \
   4KPlayer \
   Camera2

# file system
PRODUCT_PACKAGES += \
    setup_fs \
	e2fsck \
	libext2fs \
	libext2_blkid \
	libext2_uuid \
	libext2_profile \
	libext2_com_err \
	libext2_e2p \
	make_ext4fs

# preinstall apk
PRODUCT_PACKAGES += \
	DragonFire.apk \
	DragonPhone.apk

# usb
PRODUCT_PACKAGES += \
	com.android.future.usb.accessory

# audio
PRODUCT_PACKAGES += \
	audio.primary.kylin \
	audio.a2dp.default \
	audio.usb.default  \
	audio.r_submix.default 
	
PRODUCT_COPY_FILES += \
	device/softwinner/kylin-common/hardware/audio/audio_policy.conf:system/etc/audio_policy.conf \
	device/softwinner/kylin-common/hardware/audio/phone_volume.conf:system/etc/phone_volume.conf \
	device/softwinner/kylin-common/overlay/frameworks/native/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml

# camera
PRODUCT_PACKAGES += \
	camera.kylin \
	libjni_eglfence_awgallery \
	libjni_mosaic

# video
PRODUCT_PACKAGES += \
	libcedarxbase \
	libcedarxosal \
	libcedarv \
	libcedarv_base \
	libcedarv_adapter \
	libve \
	libaw_audio \
	libaw_audioa \
	libswdrm \
	libfacedetection \
	libsmileeyeblink \
	libapperceivepeople \
	libthirdpartstream \
	libcedarxsftstream \
	libion_alloc \
	libsrec_jni \
	libcnr \
	libjpgenc \
	libaw_venc \
	libaw_h264enc \
	libaw_vp8enc  \
	libI420colorconvert \
	libstagefrighthw \
	libOmxCore \
	libOmxVenc \
	libOmxVdec

PRODUCT_COPY_FILES += \
	device/softwinner/kylin-common/media_codecs.xml:system/etc/media_codecs.xml 
	
# miracast
PRODUCT_COPY_FILES += \
  device/softwinner/kylin-common/wfd_blacklist.conf:system/etc/wfd_blacklist.conf
 
# face detection
PRODUCT_COPY_FILES += \
  device/softwinner/kylin-common/facedetection/awfaceftr.aw:system/usr/share/bmd/awfaceftr.aw \
  device/softwinner/kylin-common/facedetection/awfaceftr.ftr:system/usr/share/bmd/awfaceftr.ftr \
  device/softwinner/kylin-common/facedetection/sm.awl:system/usr/share/bmd/sm.awl \
  device/softwinner/kylin-common/facedetection/ey.awl:system/usr/share/bmd/ey.awl \
  device/softwinner/kylin-common/facedetection/eb.awl:system/usr/share/bmd/eb.awl \
  device/softwinner/kylin-common/facedetection/awApperceivepeople.aw:system/usr/share/bmd/awApperceivepeople.aw \
  device/softwinner/kylin-common/facedetection/awApperceivepeople.ftr:system/usr/share/bmd/awApperceivepeople.ftr
 
# sensor
PRODUCT_COPY_FILES += \
    device/softwinner/kylin-common/sensors.sh:system/bin/sensors.sh

# init.rc, init.sun9i.usb.rc
PRODUCT_COPY_FILES += \
	device/softwinner/kylin-common/init.rc:root/init.rc \
    device/softwinner/kylin-common/init.sun9i.usb.rc:root/init.sun9i.usb.rc

PRODUCT_COPY_FILES += \
	device/softwinner/kylin-common/init_parttion.sh:root/sbin/init_parttion.sh

PRODUCT_COPY_FILES += \
	device/softwinner/kylin-common/media_codecs.xml:system/etc/media_codecs.xml \
	frameworks/native/data/etc/android.hardware.location.xml:system/etc/permissions/android.hardware.location.xml \
	frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
	frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \

PRODUCT_COPY_FILES += \
    device/softwinner/kylin-common/egl/pvrsrvctl:system/vendor/bin/pvrsrvctl \
    device/softwinner/kylin-common/egl/libusc.so:system/vendor/lib/libusc.so \
    device/softwinner/kylin-common/egl/libglslcompiler.so:system/vendor/lib/libglslcompiler.so \
    device/softwinner/kylin-common/egl/libIMGegl.so:system/vendor/lib/libIMGegl.so \
    device/softwinner/kylin-common/egl/libpvrANDROID_WSEGL.so:system/vendor/lib/libpvrANDROID_WSEGL.so \
    device/softwinner/kylin-common/egl/libPVRScopeServices.so:system/vendor/lib/libPVRScopeServices.so \
    device/softwinner/kylin-common/egl/libsrv_init.so:system/vendor/lib/libsrv_init.so \
    device/softwinner/kylin-common/egl/libsrv_um.so:system/vendor/lib/libsrv_um.so \
    device/softwinner/kylin-common/egl/libEGL_POWERVR_ROGUE.so:system/vendor/lib/egl/libEGL_POWERVR_ROGUE.so \
    device/softwinner/kylin-common/egl/libGLESv1_CM_POWERVR_ROGUE.so:system/vendor/lib/egl/libGLESv1_CM_POWERVR_ROGUE.so \
    device/softwinner/kylin-common/egl/libGLESv2_POWERVR_ROGUE.so:system/vendor/lib/egl/libGLESv2_POWERVR_ROGUE.so \
    device/softwinner/kylin-common/egl/gralloc.sunxi.so:system/vendor/lib/hw/gralloc.sun9i.so \
	device/softwinner/kylin-common/egl/powervr.ini:system/etc/powervr.ini

# setting BOARD_EGL_CFG := device/softwinner/kylin-common/egl/egl.cfg
# in BoardConfigCommon.mk instead of
#PRODUCT_COPY_FILES += \
#	device/softwinner/kylin-common/egl/egl.cfg:system/lib/egl/egl.cfg \
#

PRODUCT_PROPERTY_OVERRIDES += \
	wifi.interface=wlan0 \
	wifi.supplicant_scan_interval=15 \
	keyguard.no_require_sim=true

PRODUCT_PROPERTY_OVERRIDES += \
	ro.kernel.android.checkjni=0

PRODUCT_PROPERTY_OVERRIDES += \
	ro.opengles.version=196608
	
PRODUCT_PROPERTY_OVERRIDES += \
	persist.sys.strictmode.visual=0 \
	persist.sys.strictmode.disable=1

PRODUCT_PROPERTY_OVERRIDES += \
	ro.sys.cputype=UltraOcta-A80

#add for hwc debug
PRODUCT_PROPERTY_OVERRIDES += \
	debug.hwc.showfps=0

#add for startbg service at the same time
PRODUCT_PROPERTY_OVERRIDES += \
	ro.config.max_starting_bg=10
# Enabling type-precise GC results in larger optimized DEX files.  The
# additional storage requirements for ".odex" files can cause /system
# to overflow on some devices, so this is configured separately for
# each product.
PRODUCT_TAGS += dalvik.gc.type-precise

# if DISPLAY_BUILD_NUMBER := true then
# BUILD_DISPLAY_ID := $(BUILD_ID).$(BUILD_NUMBER)
# required by gms.
DISPLAY_BUILD_NUMBER := true
BUILD_NUMBER := $(shell date +%Y%m%d)
PRODUCT_PACKAGES += \
	PartnerBookmarksProvider

PRODUCT_PROPERTY_OVERRIDES += \
	drm.service.enabled=true

PRODUCT_PACKAGES += \
	com.google.widevine.software.drm.xml \
	com.google.widevine.software.drm \
	libdrmwvmplugin \
	libwvm \
	libWVStreamControlAPI_L3 \
	libwvdrm_L3 \
	libdrmdecrypt \
	libwvdrmengine
