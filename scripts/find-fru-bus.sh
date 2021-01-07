#!/bin/bash

confPath=/usr/share/mac-address
confFilename=config.txt

#check config file
if [ ! -f "${confPath}/${confFilename}" ]; then
    exit 0
fi

configTargetName=$(grep "eeprom=" ${confPath}/${confFilename})
if [ -z "${configTargetName}" ]; then
    exit 0
fi

targetName="${configTargetName#*eeprom=}"
targetPath=$(grep -xl "${targetName}" /sys/bus/i2c/devices/*/of_node/name)

# Get target i2c number
if [ -n "${targetPath}" ]; then
    targetI2CNum=${targetPath%-00*}
    targetI2CNum=${targetI2CNum#*devices/}

    # Modify config.txt
    if [ -n "${targetI2CNum}" ]; then
        sed -i -e 's/fruBusNum=.*/fruBusNum='"${targetI2CNum}"'/g' ${confPath}/${confFilename}
    fi
fi

