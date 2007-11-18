#!/bin/bash

MSVC_PATH=d:\\Programas\\Microsoft\ Visual\ Studio\ 8
PATH=${MSVC_PATH}/Common7/IDE:${PATH}
PATH=${MSVC_PATH}/VC/ce/bin/x86_arm:${PATH}

export PATH

INCLUDE="${MSVC_PATH}\\VC\\ce\\include"
INCLUDE="${MSVC_PATH}\\SmartDevices\\SDK\\PocketPC2003\\Include;${INCLUDE}"

export INCLUDE

cl.exe /nologo /W3 /O2 \
    /D "ARM" /D "_M_ARM" \
    /D "WIN32" /D "NDEBUG" /D "_WINDOWS" \
    /D "STRICT" /D "_WIN32_WCE=0x300" \
    /D "UNICODE" /D "_UNICODE" \
    /c msvc.cpp
