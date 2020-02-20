#!/bin/bash

touch test.txt

counter=0
while [ $counter -lt 100 ]
do
	echo "this is a test. counter = $counter" >> test.txt
	((counter++))
done
