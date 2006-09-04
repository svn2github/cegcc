#!/bin/sh
#
# Read the settings
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi
#
# Put the cleanup here instead of in the calling script
#
if [ -d $BUILD_DIR/gcc ]; then
	rm -rf $BUILD_DIR/gcc
fi
#
# cd $BUILD_DIR
# make all-gcc || exit 1
#
# Create a description of files not to put in the tar.
#
echo '*/.svn*' >/tmp/exclude-$$
echo '/*.cvs*' >>/tmp/exclude-$$
echo '*~' >>/tmp/exclude-$$
#
cd $TOP_SRCDIR
ln -s . cegcc-$CEGCC_RELEASE
tar --exclude-from=/tmp/exclude-$$ \
	-cz -f /tmp/cegcc-src-$CEGCC_RELEASE.tar.gz \
	cegcc-$CEGCC_RELEASE/NEWS cegcc-$CEGCC_RELEASE/scripts cegcc-$CEGCC_RELEASE/docs \
	cegcc-$CEGCC_RELEASE/website cegcc-$CEGCC_RELEASE/test cegcc-$CEGCC_RELEASE/src
rm cegcc-$CEGCC_RELEASE
#
# Remove temp file
#
rm -f /tmp/exclude-$$
exit 0
#
# This works
# tar --exclude-from=scripts/linux/x -cz -f /tmp/cegcc-src-0.06.tar.gz NEWS scripts docs website test
#
