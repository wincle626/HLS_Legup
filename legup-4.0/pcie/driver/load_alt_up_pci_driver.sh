#!/bin/sh

# The name of the module file you want to load into the kernel. It is determined by the Makefile which compiles this module file.
# This name will be shown inside /proc/modules file or when using the command lsmod.
DRV_MODULE="alt_up_pci"
# The name of driver you put into the kernel. It is defined in alt_up_pci_driver.h.
# This name can be found inside /proc/devices file
DRV_NAME="alt_up_pci"
# The default name of the configuration file.
CONFIG_FILE="config_file"
# This will enable the file to be read and written by everyone.
DRV_MODE="666"
# Set minor number to 0.
DRV_MINOR="0"

# Group: since distributions have different group-policy,
# look for wheel or use staff
if grep '^staff:' /etc/group > /dev/null; then
	DRV_GROUP="staff"
else
	DRV_GROUP="wheel"
fi

# check the input argument,which is the configuration file to be loaded with the driver
if [ -f "$1" ]; then
	CONFIG_FILE=$1
fi

# invoke insmod with the information extracted from the configuration file
/sbin/insmod -f ./${DRV_MODULE}.ko `awk '$1!~/^#/' $CONFIG_FILE | tr -d " " | tr -d "\t" | tr "\n" " "` ||  exit 1 

# get the major number of the device if the probe() is called successfully by the kernel
DRV_MAJOR=`cat /proc/devices | grep $DRV_NAME | awk '{print $1}' `

# check whether the driver get a major number for cdev or not
if [ -n "$DRV_MAJOR" ]; then
	echo Matching Device Found
else
	echo Matching Device Not Found 
	sudo ./unload_alt_up_pci_driver.sh
	exit 1
fi

# create character special file with the major number and the minor number defined.
# The created file is used to access the driver
mknod /dev/$DRV_NAME$DRV_MINOR c $DRV_MAJOR $DRV_MINOR

# set appropriate group and permission
chgrp $DRV_GROUP /dev/$DRV_NAME$DRV_MINOR
chmod $DRV_MODE  /dev/$DRV_NAME$DRV_MINOR