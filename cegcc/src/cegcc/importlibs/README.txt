Run dump_sdk.sh to extract the def files from the sdk
Run build.sh to rebuild the import libs.

The winsock and ws2 symbols are renamed to avoid conflicts with
newlib defined symbols. See build.sh, mkimport.sh and 
newlib/libc/sys/wince/msnet.c for details.

Conflicting symbols from coredll where hand removed from the def file.

The dumps where made against the Pocket PC 2003 SDK.

