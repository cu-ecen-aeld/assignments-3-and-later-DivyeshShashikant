#!/bin/bash

# Check for error if there are invalid arguments
if [ ! $# -eq 2 ]
then 
	echo Invalid Parameters
	exit 1

fi

# Create the path directory
mkdir -p $(dirname $1)

# Create new file with name and path
touch $1 

# Check for error if file is created or not
if [ -f $1 ]	
then
	echo "$2" > $1
	exit 0
else
	echo File Not Created
	exit 1

fi
