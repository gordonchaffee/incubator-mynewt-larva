#!/bin/bash
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
# 
#   http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#
# Called: $0 <binary> [identities...]
#  - binary is the path to prefix to target binary, .elf.bin appended to this
#    name is the raw binary format of the binary.
#  - identities is the project identities string. So you can have e.g. different
#    flash offset for bootloader identity
# 
#
if [ $# -lt 1 ]; then
    echo "Need binary to download"
    exit 1
fi

BASENAME=$1
IS_BOOTLOADER=0
BIN2IMG=project/bin2img/bin/bin2img/bin2img.elf
VER=11.22.33.44

# Look for 'bootloader' from 2nd arg onwards
shift
while [ $# -gt 0 ]; do
    if [ $1 = "bootloader" ]; then
	IS_BOOTLOADER=1
    fi
    shift
done

if [ $IS_BOOTLOADER -eq 1 ]; then
    FLASH_OFFSET=0x08000000
    FILE_NAME=$BASENAME.elf.bin
else
    FLASH_OFFSET=0x08020000
    ELF_FILE=$BASENAME.elf
    BIN_FILE=$BASENAME.elf.bin
    FILE_NAME=$BASENAME.img
    # If there is no image file, or it is older than bin file, run bin2img.
    if [ ! -f $FILE_NAME -o $FILE_NAME -ot $ELF_FILE ]; then
        echo "Version is >" $VER "<"
        $BIN2IMG $BIN_FILE $FILE_NAME $VER
        if [ "$?" -ne 0 ]; then
	    exit 1
	fi
    fi
fi
echo "Downloading" $FILE_NAME "to" $FLASH_OFFSET

openocd -f interface/ftdi/olimex-arm-usb-tiny-h.cfg -s hw/bsp/olimex_stm32-e407_devboard -f f407.cfg -c init -c "reset halt" -c "flash write_image erase $FILE_NAME $FLASH_OFFSET" -c "reset run" -c shutdown

