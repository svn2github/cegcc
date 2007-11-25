#!/bin/bash

defdir=$1
outputdir=$2

./mkimport.sh $defdir/coredll.def $outputdir || exit
./mkimport.sh $defdir/note_prj.def $outputdir || exit
./mkimport.sh $defdir/iphlpapi.def $outputdir || exit
./mkimport.sh $defdir/commctrl.def $outputdir || exit
./mkimport.sh $defdir/ole32.def $outputdir || exit
./mkimport.sh $defdir/oleaut32.def $outputdir || exit
./mkimport.sh $defdir/aygshell.def $outputdir || exit
./mkimport.sh $defdir/ceshell.def $outputdir || exit
./mkimport.sh $defdir/winsock.def $outputdir || exit
./mkimport.sh $defdir/ws2.def $outputdir || exit
./mkimport.sh $defdir/mmtimer.def $outputdir || exit
./mkimport.sh $defdir/toolhelp.def $outputdir || exit
#./winsock.sh $defdir $outputdir || exit
#./ws2.sh $defdir $outputdir 
