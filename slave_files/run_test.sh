#!/bin/bash

echo "create storage"
touch backup_storage

echo "create plugins dir"
mkdir ./plugins

echo "making slave"
make slave

echo "run slave: ./slave_main.out"
./slave_main.out

echo "cleaning"
rm backup_storage
rm -r ./plugins/

# NOTE: the path "/usr/local/lib" was added to a file named "/etc/ld.so.conf",
# then a command "sudo ldconfig" was entered.
# this allowed my program to find and link the libconfig files.
