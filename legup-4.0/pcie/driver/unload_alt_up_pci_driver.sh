#!/bin/sh

DRV_MODULE="alt_up_pci"
DRV_NAME="alt_up_pci"

/sbin/rmmod -f $DRV_MODULE

# remove state nodes
rm -f /dev/${DRV_NAME}*

