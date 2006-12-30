#!/bin/sh
if [ -x ./rpm-create-source.sh ]; then
	cd ../..
	TOP_SRCDIR=`pwd`
else
	TOP_SRCDIR=`pwd`
fi
#
export TOP_SRCDIR
#
# The name of this release
#
CEGCC_RELEASE=0.12
export CEGCC_RELEASE
#
# Don't take unnecessary stuff in the source file.
#
echo '*/.svn*' >/tmp/exclude-$$
echo '/*.cvs*' >>/tmp/exclude-$$
echo '*/CVS' >>/tmp/exclude-$$
echo '*/CVS/*' >>/tmp/exclude-$$
echo 'cegcc-'$CEGCC_RELEASE'/src/build-cegcc' >>/tmp/exclude-$$
echo 'cegcc-'$CEGCC_RELEASE'/src/build-cegcc/*' >>/tmp/exclude-$$
echo 'cegcc-'$CEGCC_RELEASE'/src/build-mingw32ce' >>/tmp/exclude-$$
echo 'cegcc-'$CEGCC_RELEASE'/src/build-mingw32ce/*' >>/tmp/exclude-$$
echo '*~' >>/tmp/exclude-$$
#
cd $TOP_SRCDIR
ln -s . cegcc-$CEGCC_RELEASE
# tar --exclude-from=/tmp/exclude-$$ \
# 	-cz -f /usr/src/rpm/SOURCES/cegcc-src-$CEGCC_RELEASE.tar.gz \
# 	cegcc-$CEGCC_RELEASE/NEWS cegcc-$CEGCC_RELEASE/README \
# 	cegcc-$CEGCC_RELEASE/scripts cegcc-$CEGCC_RELEASE/docs \
# 	cegcc-$CEGCC_RELEASE/website cegcc-$CEGCC_RELEASE/test cegcc-$CEGCC_RELEASE/src
rm cegcc-$CEGCC_RELEASE
#
# Remove temp file
#
rm -f /tmp/exclude-$$
#
# Tell packager what to do.
#
echo "Now be sure to build with"
echo " "
echo "	rm -rf /opt/cegcc/* /opt/mingw32ce/*"
echo "	rpmbuild -tb /usr/src/rpm/SOURCES/cegcc-src-"$CEGCC_RELEASE".tar.gz"
echo " "
#
# All done
#
exit 0
