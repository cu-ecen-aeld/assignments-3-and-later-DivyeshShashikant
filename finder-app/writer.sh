#!/bin/bash

if [ ! $# -eq 2 ]
then 
	echo Invalid Parameters
	exit 1

fi

mkdir -p $(dirname $1)
touch $1 
if [ -f $1 ]	
then
	echo "$2" > $1
	exit 0
else
	echo File Not Created
	exit 1

fi
