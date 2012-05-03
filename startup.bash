#! /bin/bash
#
# MurkMUD++ - A Windows compatible, C++ compatible Merc 2.2 Mud.
#
# author Jon A. Lambert
# date 01/02/2007
# version 1.5
# remarks
#  This source code copyright (C) 2005, 2006, 2007 by Jon A. Lambert
#  All rights reserved.
#
#  Use governed by the MurkMUD++ public license found in license.murk++

# Set the port number.
port=4000
if [ -n "$1" ]
then
  port=$1
fi

if [ -e "shutdown.txt" ]
then
  rm -f shutdown.txt
fi  

while [ 1 = 1 ]
do
    index=1000
    while [ 1 = 1 ]
    do
	logfile="./${index}.log"
	if [ ! -e $logfile ] 
	then 
	  break
	fi  
	let index+=1
    done

    # Record starting time
    date > $logfile

    # Run mud.
    ./murk $port > $logfile 2>&1 

    # Restart, giving old connections a chance to die.
    if [ -e shutdown.txt ]
    then
	rm -f shutdown.txt
	exit 0
    fi
    sleep 15
done
