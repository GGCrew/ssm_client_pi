Expand File System
Change Locale (en.US UTF-8)
Change Timezone (America - Pacific)
Change Keyboard (Generic 101, US)
Change Memory Split to 256MB


sudo apt-get update && sudo apt-get upgrade
sudo apt-get install subversion vim htop
	


vim ~/update_ssm_client
	svn export --force svn://net-night.com/snapshowmagic/R_Pi/snap_show_magic/tag/ssm_client ./
	chmod +x ssm_client

	mkdir ./images
	svn export --force svn://net-night.com/snapshowmagic/R_Pi/snap_show_magic/tag/images/black.png ./images/

	svn export --force svn://net-night.com/snapshowmagic/R_Pi/snap_show_magic/tag/install.sh ./
	chmod +x install.sh

	sudo ./install.sh
chmod +x update_ssm_client
./update_ssm_client


sudo vim /etc/network/interfaces
	auto lo

	iface lo inet loopback
	iface eth0 inet dhcp

	allow-hotplug wlan0
	iface wlan0 inet manual
	wpa-roam /etc/wpa_supplicant/wpa_supplicant.conf
	iface default inet dhcp

sudo vim /etc/wpa_supplicant/wpa_supplicant.conf
	ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
	update_config=1

	#network={
	#	ssid=""
	#	scan_ssid=1
	#	proto=RSN
	#	key_mgmt=WPA-PSK
	#	pairwise=CCMP TKIP
	#	group=CCMP TKIP
	#	psk=""
	#}

	network={
		ssid="SnapShowMagic"
		scan_ssid=1
		key_mgmt=NONE
	}

sudo ifdown wlan0 && sudo ifup wlan0



sudo vim /etc/init.d/ssm_client
	#! /bin/sh
	# /etc/init.d/ssm_client

	# The following part always gets executed.
	#echo "This part always gets executed"

	# The following part carries out specific functions depending on arguments.
	case "$1" in
		start)
			echo "Starting ssm_client"
			#echo "ssm_client is alive"
			su - pi -c "/usr/local/bin/ssm_client"
			;;
		stop)
			echo "Stopping ssm_client"
			echo "ssm_client is dead"
			;;
		*)
			echo "Usage: /etc/init.d/ssm_client {start|stop}"
			exit 1
			;;
	esac

	exit 0
sudo chmod a+x /etc/init.d/ssm_client
sudo update-rc.d ssm_client defaults



