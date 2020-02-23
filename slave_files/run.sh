#!/bin/bash

echo "create storage"
touch storage

echo "create plugins dir"
mkdir ./plugins

echo "run slave: ./cloudio_slave.out"
./cloudio_slave.out



# NOTE: the path "/usr/local/lib" was added to a file named "/etc/ld.so.conf",
# then a command "sudo ldconfig" was entered.
# this allowed my program to find and link the libconfig files.
