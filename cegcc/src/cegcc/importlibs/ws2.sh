#!/bin/bash

def=$1
outputdir=$2

syms='accept bind closesocket connect gethostbyaddr 
		gethostbyname gethostname getpeername getsockname getsockopt
		ioctlsocket listen recv recvfrom select send
		sendto setsockopt shutdown socket'

./mkimport.sh $def/ws2.def $outputdir $syms
