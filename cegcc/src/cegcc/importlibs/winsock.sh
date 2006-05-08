#!/bin/bash

if [ $# -lt 2 ] ; then
	echo "usage:"
	echo "$0 [input defs dir] [output directory]"
	exit 1
fi

def=$1
outputdir=$2

if [ ! -d $def ]
then
	echo "error: def files directory \"$def\" not found."
	exit 1
fi

if [ ! -d $outputdir ]
then
	echo "error: output directory \"$outputdir\" not found."
	exit 1
fi


syms='accept bind closesocket connect gethostbyaddr
		gethostbyname gethostname getpeername getsockname getsockopt
		ioctlsocket listen recv recvfrom select send 
		sendto setsockopt shutdown socket'

./mkimport.sh $def/winsock.def $outputdir $syms
