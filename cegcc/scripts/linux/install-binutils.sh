#!/bin/sh
cd $BUILD_DIR/binutils
make install || exit 1
exit 0
