# 3G Data Card Packages
PRODUCT_PACKAGES += \
	u3gmonitor \
	chat \
	rild \
	pppd

# 3G Data Card Configuration Flie
PRODUCT_COPY_FILES += \
	device/softwinner/kylin-common/rild/ip-down:system/etc/ppp/ip-down \
	device/softwinner/kylin-common/rild/ip-up:system/etc/ppp/ip-up \
	device/softwinner/kylin-common/rild/3g_dongle.cfg:system/etc/3g_dongle.cfg \
	device/softwinner/kylin-common/rild/usb_modeswitch:system/bin/usb_modeswitch \
	device/softwinner/kylin-common/rild/call-pppd:system/xbin/call-pppd \
	device/softwinner/kylin-common/rild/usb_modeswitch.sh:system/xbin/usb_modeswitch.sh \
	device/softwinner/kylin-common/rild/apns-conf_sdk.xml:system/etc/apns-conf.xml \
	device/softwinner/kylin-common/rild/libsoftwinner-ril-4.4.so:system/lib/libsoftwinner-ril-4.4.so

#PRODUCT_COPY_FILES += \
#	device/softwinner/kylin-common/rild/init.3gdongle.rc:root/init.sun9i.3gdongle.rc

# 3G Data Card usb modeswitch File
PRODUCT_COPY_FILES += \
	$(call find-copy-subdir-files,*,device/softwinner/kylin-common/rild/usb_modeswitch.d,system/etc/usb_modeswitch.d)
	
PRODUCT_PROPERTY_OVERRIDES += \
		    ro.sw.embeded.telephony = false
