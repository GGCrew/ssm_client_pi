#! /bin/sh

echo Installing the binary and make it executable...
cp --force ssm_client /usr/local/bin
chmod 755 /usr/local/bin/ssm_client

echo Creating data folders...
mkdir --parents "/usr/local/var/snapshowmagic/images"
mkdir --parents "/usr/local/var/snapshowmagic/downloads"
chmod -R a+rwX "/usr/local/var/snapshowmagic/downloads"

echo Installing essential data files...
# (The default black image could, arguably, belong in datarootdir because it is never modified by the program.)
cp ./images/black.png "/usr/local/var/snapshowmagic/images"
touch "/usr/local/var/snapshowmagic/server_history"
chmod a+rw "/usr/local/var/snapshowmagic/server_history"

echo Done!
