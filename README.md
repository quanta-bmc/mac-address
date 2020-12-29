# mac-address

## Error Handling

*   If the config file corrupts, NO MAC address would be generated.
*   If the FRU data corrupts, use random MAC addresses instead.

## Find FRU automatically
The scripts/find-fru-bus.sh can find specified FRU according to the eeprom in config.txt.
This script gets back FRU's bus number as well as address and update to config.txt.

## Config File Format

The config file is in `/usr/share/mac-address/config.txt`.

The followings are items needed for the config file.

*   `eeprom`: This is the node name(of_node/name) of FUR's EEPROM.
*   `fruBusNum`: This is the i2c bus number of FRU that stores MAC
    address info.
*   `fruAddr`: This is FRU address that stores MAC address info.
*   `numberMac`: This represents the number of MAC addresses.
*   `mac<number>`: This contains MAC address name.

Note that `usb0_dev` and `usb0_host` are for USB ports.

Here is an example.
```
eeprom=fru_1
fruBusNum=1
fruAddr=0x01
numberMac=4
mac1=eth0
mac2=eth1
mac3=usb0_dev
mac4=usb0_host
```
