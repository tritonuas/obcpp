#!/bin/bash

sudo ifconfig eth0 mtu 9000

sudo ethtool -g eth0 
sudo ethtool -G eth0 rx 2048 

sudo sh -c "echo 'net.core.rmem_default=1048576' >> /etc/sysctl.conf"
sudo sh -c "echo 'net.core.rmem_max=1048576' >> /etc/sysctl.conf"
sudo sysctl -p

sudo sysctl -w net.ipv4.conf.default.rp_filter=0
sudo sysctl -w net.ipv4.conf.all.rp_filter=0
sudo sysctl -w net.ipv4.conf.eth0.rp_filter=0

sudo ifconfig eth0 169.254.0.1 netmask 255.255.0.0

# also need to disable reverse path filtering
# https://support.thinklucid.com/arena-sdk-documentation/#4702
# Either run the following commands:
#    sudo sysctl -w net.ipv4.conf.default.rp_filter=0
#    sudo sysctl -w net.ipv4.conf.all.rp_filter=0
#    sudo sysctl -w net.ipv4.conf.enp0s8.rp_filter=0
# Or comment out lines that look like the following in /etc/sysctl.d/10-network-security.conf. 
# This will make these changes persistent.
#    net.ipv4.conf.default.rp_filter=1
#    net.ipv4.conf.all.rp_filter=1


exit 0