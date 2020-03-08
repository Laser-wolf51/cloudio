#!/bin/bash

echo "create storage"
touch backup_storage

echo "create plugins dir"
mkdir ./plugins

echo "run slave: ./slave_main.out"
./slave_main.out


