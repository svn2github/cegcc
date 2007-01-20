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
echo 'cegcc-cegcc-'$CEGCC_RELEASE'/src/build-cegcc' >>/tmp/exclude-$$
echo 'cegcc-cegcc-'$CEGCC_RELEASE'/src/build-cegcc/*' >>/tmp/exclude-$$
echo 'cegcc-cegcc-'$CEGCC_RELEASE'/src/build-mingw32ce' >>/tmp/exclude-$$
echo 'cegcc-cegcc-'$CEGCC_RELEASE'/src/build-mingw32ce/*' >>/tmp/exclude-$$
echo 'cegcc-mingw32ce-'$CEGCC_RELEASE'/src/build-cegcc' >>/tmp/exclude-$$
echo 'cegcc-mingw32ce-'$CEGCC_RELEASE'/src/build-cegcc/*' >>/tmp/exclude-$$
echo 'cegcc-mingw32ce-'$CEGCC_RELEASE'/src/build-mingw32ce' >>/tmp/exclude-$$
echo 'cegcc-mingw32ce-'$CEGCC_RELEASE'/src/build-mingw32ce/*' >>/tmp/exclude-$$
echo '*~' >>/tmp/exclude-$$
#
cd $TOP_SRCDIR
ln -s . cegcc-cegcc-$CEGCC_RELEASE
tar --exclude-from=/tmp/exclude-$$ \
	-cz -f /usr/src/rpm/SOURCES/cegcc-cegcc-src-$CEGCC_RELEASE.tar.gz \
	cegcc-cegcc-$CEGCC_RELEASE/NEWS \
	cegcc-cegcc-$CEGCC_RELEASE/README \
	cegcc-cegcc-$CEGCC_RELEASE/scripts/linux/cegcc.spec \
	cegcc-cegcc-$CEGCC_RELEASE/docs \
	cegcc-cegcc-$CEGCC_RELEASE/website \
	cegcc-cegcc-$CEGCC_RELEASE/test \
	cegcc-cegcc-$CEGCC_RELEASE/src
rm cegcc-cegcc-$CEGCC_RELEASE
#
echo "Ready to build with"
echo " "
echo "	rm -rf /opt/cegcc/*"
echo "	rpmbuild -tb /usr/src/rpm/SOURCES/cegcc-cegcc-src-"$CEGCC_RELEASE".tar.gz"
echo "Note : " `ls -l /usr/src/rpm/SOURCES/cegcc-cegcc-src-"$CEGCC_RELEASE".tar.gz`
#
ln -s . cegcc-mingw32ce-$CEGCC_RELEASE
tar --exclude-from=/tmp/exclude-$$ \
	-cz -f /usr/src/rpm/SOURCES/cegcc-mingw32ce-src-$CEGCC_RELEASE.tar.gz \
	cegcc-mingw32ce-$CEGCC_RELEASE/NEWS \
	cegcc-mingw32ce-$CEGCC_RELEASE/README \
	cegcc-mingw32ce-$CEGCC_RELEASE/scripts/linux/mingw32ce.spec \
	cegcc-mingw32ce-$CEGCC_RELEASE/docs \
	cegcc-mingw32ce-$CEGCC_RELEASE/website \
	cegcc-mingw32ce-$CEGCC_RELEASE/test \
	cegcc-mingw32ce-$CEGCC_RELEASE/src
rm cegcc-mingw32ce-$CEGCC_RELEASE
#
# Remove temp file
#
rm -f /tmp/exclude-$$
#
# Tell packager what to do.
#
echo " "
echo "	rm -rf /opt/mingw32ce/*"
echo "	rpmbuild -tb /usr/src/rpm/SOURCES/cegcc-mingw32ce-src-"$CEGCC_RELEASE".tar.gz"
echo "Note : " `ls -l /usr/src/rpm/SOURCES/cegcc-mingw32ce-src-"$CEGCC_RELEASE".tar.gz`
echo " "
#
# All done
#
exit 0
