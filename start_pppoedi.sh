#!/bin/bash

if [[ "$1" = "-h" ]]; then

	echo -e "\nThis script makes sure every minute that PPPoEDI is up and running."
	echo -e "It is intended to run at startup boot on systemd Linux.\n"
	echo -e "Prerequisite:\n"
	echo -e "       Install PPPoEDI from https://github.com/LAR-UFES/pppoe-di"
	exit
fi


while [ 1 ]
do
	PPP=$(ifconfig | grep "UP POINTOPOINT RUNNING")
	if ! [ -n "$PPP" ]; then
		/usr/bin/poff
		/usr/bin/pon lar
	fi
	sleep 60
done

