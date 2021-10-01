#!/bin/bash

confPath=/usr/share/mac-address
confFilename=config.txt

of_name_to_eeproms() {
  local names
  if ! names="$(grep -xl "$1" /sys/bus/i2c/devices/*/of_node/name)"; then
    echo "Failed to find eeproms with of_name '$1'" >&2
    return 1
  fi
  names=$(echo "$names" | sed 's,/of_node/name$,/eeprom,')
  if (( "$(echo "$names" | wc -l)" != 1 )); then
    for path in ${names[@]}
    do
        if [ -f "$path" ]; then
          exist_path="$exist_path $path"
        fi
    done
    names=$exist_path
  fi
  eval echo "$names"
}

#check config file
if [ ! -f "${confPath}/${confFilename}" ]; then
    exit 0
fi

configTargetName=$(grep "eeprom=" ${confPath}/${confFilename})
if [ -z "${configTargetName}" ]; then
    exit 0
fi

targetName="${configTargetName#*eeprom=}"
targetPath=$(of_name_to_eeproms "${targetName}")

# Get target i2c number
if [ -n "${targetPath}" ]; then
    targetI2CNum=${targetPath%-00*}
    targetI2CNum=${targetI2CNum#*devices/}

    # Modify config.txt
    if [ -n "${targetI2CNum}" ]; then
        sed -i -e 's/fruBusNum=.*/fruBusNum='"${targetI2CNum}"'/g' ${confPath}/${confFilename}
    fi
fi

