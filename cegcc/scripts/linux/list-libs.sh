#!/bin/sh
#
# Set up a list of DLL proxies to make.
#
BUILD_LIBS_LIST="coredll winsock aygshell commctrl iphlpapi ws2 ceshell"
INSTALL_PE_TGT="$BUILD_LIBS_LIST"
INSTALL_MINGW_TGT="$BUILD_LIBS_LIST"
# LIST="coredll winsock aygshell commctrl fileopen"
#
# List of libraries to squash into once libcegcc.a
#
LIBS_CEGCC_LIB="c winsock"
