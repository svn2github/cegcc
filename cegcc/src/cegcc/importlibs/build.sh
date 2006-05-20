#!/bin/bash

defdir=$1
outputdir=$2

./mkimport.sh $defdir/coredll.def $outputdir && \
./mkimport.sh $defdir/icmplib.def $outputdir && \
./mkimport.sh $defdir/note_prj.def $outputdir && \
./mkimport.sh $defdir/iphlpapi.def $outputdir && \
./mkimport.sh $defdir/commctrl.def $outputdir && \
./mkimport.sh $defdir/ole32.def $outputdir && \
./mkimport.sh $defdir/oleaut32.def $outputdir && \
./mkimport.sh $defdir/aygshell.def $outputdir && \
./mkimport.sh $defdir/ceshell.def $outputdir && \
./winsock.sh $defdir $outputdir && \
./ws2.sh $defdir $outputdir 
