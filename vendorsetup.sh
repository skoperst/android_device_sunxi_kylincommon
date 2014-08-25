#
# Copyright (C) 2008 The Android Open Source Project
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

# This file is executed by build/envsetup.sh, and can use anything
# defined in envsetup.sh.
#
# In particular, you can add lunch options with the add_lunch_combo
# function: add_lunch_combo generic-eng

#!/bin/bash

function cdevice()
{	
	cd $DEVICE
}

function cout()
{
	cd $OUT	
}

function extract-bsp()
{
	LICHEE_DIR=$ANDROID_BUILD_TOP/../lichee
	LINUXOUT_DIR=$LICHEE_DIR/out/sun9iw1p1/android/common
	LINUXOUT_MODULE_DIR=$LINUXOUT_DIR/lib/modules/*/*
	CURDIR=$PWD

	cd $DEVICE

	#extract kernel
	if [ -f kernel ] ; then
		rm kernel
	fi
	cp $LINUXOUT_DIR/bImage kernel
	echo "$DEVICE/bImage copied!"

	#extract linux modules
	if [ -d modules ] ; then
		rm -rf modules
	fi
	mkdir -p modules/modules
	cp -rf $LINUXOUT_MODULE_DIR modules/modules
	echo "$DEVICE/modules copied!"
	chmod 0755 modules/modules/*

# create modules.mk
(cat << EOF) > ./modules/modules.mk 
# modules.mk generate by extract-files.sh, do not edit it.
PRODUCT_COPY_FILES += \\
	\$(call find-copy-subdir-files,*,\$(LOCAL_PATH)/modules,system/vendor/modules)
EOF

	cd $CURDIR
}

function pack()
{
	T=$(gettop)
	export ANDROID_IMAGE_OUT=$OUT
	export PACKAGE=$T/../lichee/tools/pack

	sh $DEVICE/package.sh $*
}

function get_uboot()
{
    pack $*
    if [ ! -e $OUT/boot-resource ] ; then
        mkdir $OUT/boot-resource
    fi
    rm -rf $OUT/boot-resource/*
    cp -v $PACKAGE/out/boot-resource.fex $OUT
    cp -r $PACKAGE/out/boot-resource/* $OUT/boot-resource/
    echo "\"$PACKAGE/out/boot-resource/* -> $OUT/boot-resource/\""
    cp -v $PACKAGE/out/env.fex $OUT
    cp -v $PACKAGE/out/boot0_nand.fex $OUT
    cp -v $PACKAGE/out/boot0_sdcard.fex $OUT

    cp -v $PACKAGE/out/u-boot.fex $OUT
}
