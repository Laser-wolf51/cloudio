#!/bin/bash

# test for the application

### AVOID CLOSING THE PROG WITHOUT UNMOUNT ###

echo "creating nbd devices"
sudo modprobe nbd
echo "config the nbd to size of 4 kb"
echo 4 | sudo tee /sys/block/nbd0/queue/max_sectors_kb;

echo "create plugins dir"
mkdir ./plugins
echo "calling make"
make master

sudo gnome-terminal -x sh -c '
sleep 30
echo "creating ext4:"
sudo mkfs.ext4 /dev/nbd0
echo "mkdir mnt"
mkdir mnt
echo "mount mnt:"
sudo mount /dev/nbd0 ./mnt
echo "change permission to mnt:"
sudo chmod 777 mnt/
echo "and now test:"
cd mnt
echo "creating a test.txt file and write into it..."
../test_script.sh
cd ..
echo "done."
exec bash'

# sudo gnome-terminal -x sh -c '
# sleep 17
# echo "creating ext4:"
# sudo mkfs.ext4 /dev/nbd0
# exec bash'

echo "run: ./cloudio_master.out /dev/nbd0"
sudo ./cloudio_master.out /dev/nbd0;

# test if having troubles:
# enter: dd if=/dev/zero of=/dev/nbd0 bs=1b count=1 oflag=dsync

# NOTE: the path "/usr/local/lib" was added to a file named "/etc/ld.so.conf",
# then a command "sudo ldconfig" was entered.
# this allowed my program to find and link the libconfig files.
