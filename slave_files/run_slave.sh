#!/bin/bash
# above: 'shebang'. specify what program should run this script.

echo "create storage"
touch storage

echo "create plugins dir"
mkdir ./plugins

echo "making slave"
make slave

echo "run slave: ./cloudio_slave.out"
./cloudio_slave.out

echo "cleaning"
rm storage
rm -r ./plugins/
