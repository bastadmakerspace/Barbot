# SD card setup
# expand SD card
sudo raspi-config --expand-rootfs
sudo apt-get update && sudo apt-get dist-upgrade

# fix usb wifi dongle if there is one
#sudo ln -s /dev/null /etc/systemd/network/99-default.link

sudo nano /etc/hostname
sudo nano /etc/hosts


# install ino for arduino building
sudo apt-get install -y arduino picocom
sudo pip install ino


# if default env is python3/pip3
#sudo apt-get install python-pip
#pip2 install ino

# remove ssh warning
sudo apt purge libpam-chksshpwd

# install python
pip install pyserial

# install node/npm
sudo apt-get install nodejs npm
# go to node folder
npm install


# run on screen
export DISPLAY=:0.0

# depending on rotation, set /boot/config.txt
# for 180 inverted screen
dtoverlay=hyperpixel4
dtparam=touchscreen-swapped-x-y
dtparam=touchscreen-inverted-x
display_rotate=1


# check if wifi power management is disabled
dmesg | grep brcm
#turn off wifi power safe on boot and disable hdmi
sudo nano /etc/rc.local
/sbin/iw dev wlan0 set power_save off
/usr/bin/tvservice -o

#enable hdmi 
/usr/bin/tvservice -p

#disable
sudo update-rc.d watchdog disable
sudo service watchdog stop

# set tmpfs
sudo nano /etc/fstab
# add
tmpfs    /tmp    tmpfs    defaults,noatime,nosuid,size=100m    0 0
tmpfs    /var/log    tmpfs    defaults,noatime,nosuid,mode=0755,size=100m    0 0

# or
tmpfs /tmp tmpfs defaults,noatime,nosuid 0 0
tmpfs /var/log tmpfs defaults,noatime,nosuid,size=64m 0 0
