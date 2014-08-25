#!/sbin/busybox sh

BUSYBOX="/sbin/busybox"

mkdir /bootloader
mount -t vfat /dev/block/by-name/bootloader /bootloader

if [ ! -e /data/system.notfirstrun ] ; then
    echo "do preinstall job"

    /system/bin/sh /system/bin/pm preinstall /system/preinstall
    /system/bin/sh /system/bin/pm preinstall /sdcard/preinstall

    cp /system/preinstall/chrome-command-line /data/local/chrome-command-line
    chmod 644 /data/local/chrome-command-line

    $BUSYBOX touch /data/system.notfirstrun

    echo "preinstall ok"
else
    echo "do nothing"
fi

umount /bootloader
rmdir /bootloader
