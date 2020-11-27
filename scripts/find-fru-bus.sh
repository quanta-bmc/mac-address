#!/bin/bash

confPath=/usr/share/mac-address
confFilename=config.txt

#check config file
if [ ! -f "${confPath}/${confFilename}" ]; then
    exit 0
fi

tmp=$(grep "eeprom=" ${confPath}/${confFilename})
if [ -z "${tmp}" ]; then
    exit 0
fi

targetName="${tmp#*eeprom=}"
targetPath=$(grep -xl "${targetName}" /sys/bus/i2c/devices/*/of_node/name)

# Get target i2c number
if [ -n "${targetPath}" ]; then
    targetI2cNo=${targetPath%-00*}
    targetI2cNo=${targetI2cNo#*devices/}

    # Modify config.txt
    if [ -n "${targetI2cNo}" ]; then
        sed -i -e 's/fruBusNum=.*/fruBusNum='"${targetI2cNo}"'/g' ${confPath}/${confFilename}
    fi
fi

