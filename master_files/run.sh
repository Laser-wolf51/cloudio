#!/bin/bash

echo "creating nbd devices"
sudo modprobe nbd
echo "config the nbd to size of 4 kb"
echo 4 | sudo tee /sys/block/nbd1/queue/max_sectors_kb;

# run the application from a new terminal
sudo gnome-terminal --tab -- bash -c '
echo "run: ./master_main.out /dev/nbd1"
sudo ./master_main.out /dev/nbd1
echo "unmounting...."
sudo umount ./cloud
exec bash'

# defining a function to prepare the app
function connect()
{
echo "Have all the slaves been connected...?[y/n]"
read answer
if [ $answer != "y" ]
then
	echo "your answer wasn't 'y'"
	return 1
fi

echo "creating ext4:"
if ! sudo mkfs.ext4 /dev/nbd1
then
	echo "failed creating ext4"
	return 1
fi

echo "mkdir cloud"
mkdir cloud
# if fails, it only means the folder allready exsists.

if ! sudo mount /dev/nbd1 ./cloud
then
	echo "failed mounting"
	return 1
fi

echo "changing directory permissions"
sudo chmod 777 cloud

echo "DONE."
}

# now executing it
connect



