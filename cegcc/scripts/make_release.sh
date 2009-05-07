#!/bin/sh
#
# This release script contains some magic to restart where it failed or was aborted.
# Remove the make_release.status file to restart from the beginning, or to create
# a new release. (That file also contains the release id.)
#
# PASSED is a global variable to inspect success or failure.
# RESTARTED keeps track of where we were.
#
PASSED=yes
RESTARTED=0
#
# Make sure not to get into trouble
#
LANG=C
export LANG
#
# Status file
#
ORIGDIR=`pwd`
STATUS=$ORIGDIR/make_release.status
#
# These are fixed
#
SVN_TRUNK="https://cegcc.svn.sourceforge.net/svnroot/cegcc/trunk/cegcc"
SVN_TAGS="https://cegcc.svn.sourceforge.net/svnroot/cegcc/tags"
#
# Functions
#
function report_status()
{
	echo 'PASSED='$PASSED >>$STATUS
	echo 'RESTARTED='$RESTARTED >>$STATUS
	echo 'VERSION='$VERSION >>$STATUS
	echo 'UNAME='$UNAME >>$STATUS
	echo 'LINUXDISTRO='$LINUXDISTRO >>$STATUS
	echo 'HAS_RPM='$HAS_RPM >>$STATUS
	echo 'ARCH='$ARCH >>$STATUS
	echo 'TS1='$TS1 >>$STATUS
	echo 'TS2='$TS2 >>$STATUS
	echo 'TRUNKDIR='$TRUNKDIR >>$STATUS
	echo 'TAGSDIR='$TAGSDIR >>$STATUS
}
#
# End functions
#
#
# Get status from file
#
if test -f $STATUS
then
	RESTARTED=1
#
	echo "Reading $STATUS file"
#
	. $STATUS
	if test $RESTARTED -ne 0
	then
		echo "This is a restart run of $0, previous level was $RESTARTED"
		echo "Interrupt NOW to abort (4s)"
		sleep 4
	fi
else
	echo "No $STATUS file present"
fi

if test $RESTARTED -ge 1
then
	echo "Skip level 0 (checks)"
else
#
#	Level 0 (checks)
#
	echo "Level 0 (checks)"
	if [ -r ./NEWS -a -r scripts/linux/cegcc.spec -a -r scripts/linux/rpm-create-source.sh ]; then
		TOPDIR=.
	else
		if [ -f ../NEWS -a -r linux/cegcc.spec.in -a -r linux/rpm-create-source.sh.in ]; then
			TOPDIR=..
		else
			echo "Cannot find CeGCC source directory, exiting"
			if [ ! -r .svn/dir-wcprops ]; then
				echo "We're not even in a SVN directory !"
			fi
			exit 1
		fi
	fi
	#
	# Are we in an SVN directory ?
	#
	if [ ! -r .svn/dir-wcprops ]; then
		echo "We're not in a SVN directory, terminating"
		exit 1
	fi
	TRUNKDIR=$TOPDIR/..
	TAGSDIR=$TRUNKDIR/../tags
	#
	if [ ! -d $TAGSDIR ]; then
		echo "We're under SVN but something's still spooky, there's no tags directory"
		echo "Exiting"
		exit 1
	fi
	#
	# Do we have a release id yet ?
	#
	if [ "x$1" = "x" ] ; then
		echo "Please specify a release id as command line argument to this script"
		exit 1
	fi
	VERSION=$1
	#
	# If we're here then the tag shouldn't be present yet.
	#
	if [ -x $TAGSDIR/cegcc-$VERSION ]; then
		echo "There is already a tag called cegcc-$VERSION in SVN, aborting"
		exit 1
	fi
	#
	#	End level 0 (checks)
	#
	RESTARTED=10
	report_status
	#
	# If this is a Linux system, figure out which distribution.
	# FIX ME
	#
	# The idea is to have several variables to figure out what to do and what
	# to call the names of the files we produce.
	#
	# LINUXDISTRO can be used as part of file names, so e.g. instead of
	#	cegcc-mingw32ce-0.15-1.i586.rpm
	# we could have
	#	cegcc-mingw32ce-0.15-1.i586-mandriva.rpm
	# or	mandriva-cegcc-mingw32ce-0.15-1.i586.rpm
	#
	UNAME=`uname`
	if [ "x$UNAME" = "xLinux" ]; then
		ARCH="linux"
		if [ -x /usr/sbin/rpmdrake ]; then
			LINUXDISTRO="mandriva"
		else
			LINUXDISTRO=""
		fi
		if [ -x /usr/bin/rpm ]; then
			HAS_RPM="yes"
		else
			HAS_RPM="no"
		fi
	else
		true;
	fi
fi

#
# Level 10
#
if test $RESTARTED -ge 11
then
	echo "Skip level 10 (create source tag in SVN)"
else
	echo "Level 10 (create source tag in SVN)"
	# echo "Warning: this may take a while ..."
	TS1=`date +%Y-%m-%d-%H-%M-%s`
	echo "svn copy $SVN_TRUNK $SVN_TAGS/cegcc-$VERSION"
	OK=no
	svn copy -m "Create tag $VERSION by make_release.sh" $SVN_TRUNK $SVN_TAGS/cegcc-$VERSION && OK=yes
	TS2=`date +%Y-%m-%d-%H-%M-%s`
	if [ $OK = "no" ]; then
		echo "Creating the new tag in SVN failed, exiting ..."
		exit 1
	fi
	#
	#	End level 10 (create source tag in SVN)
	#
	RESTARTED=20
	report_status
	#
fi

#
# Level 20
#
if test $RESTARTED -ge 21
then
	echo "Skip level 20 (checkout new tag from SVN)"
else
	echo "Level 20 (checkout new tag from SVN)"
	cd $ORIGDIR
	cd $TAGSDIR
	if [ -x cegcc-$VERSION ]; then
		echo "Version cegcc-$VERSION already exists in $TAGSDIR"
		echo "Exiting .."
		exit 1
	fi
	TS1=`date +%Y-%m-%d-%H-%M-%s`
	echo "Warning: this may take a while ..."
	echo "svn update cegcc-$VERSION"
	OK=no
	svn -q update cegcc-$VERSION && OK=yes
	TS2=`date +%Y-%m-%d-%H-%M-%s`
	if [ $OK = "no" ]; then
		echo "Checkout from SVN failed, exiting ..."
		exit 1
	fi
	#
	#	End level 20 (checkout new tag from SVN)
	#
	RESTARTED=21
	report_status
	#
fi

#
# Level 21
#
if test $RESTARTED -ge 22
then
	echo "Skip level 21 (create version include file)"
else
	echo "Level 21 (create version include file)"
	OK=no
	TS1=`date +%Y%m%d`
	cd $ORIGDIR
	cd $TAGSDIR/cegcc-$VERSION/src/w32api/include
	CEGCC_VERSION_MAJOR=`echo $VERSION | awk -F. '{print $1}'`
	CEGCC_VERSION_MINOR=`echo $VERSION | awk -F. '{print $2}'`
	CEGCC_VERSION_PATCHLEVEL=`echo $VERSION | awk -F. '{print $3}'`
	L1=`grep -s -n "Automatic changes below" cegcc.h.in | awk -F: '{print $1}'`
	L2=`grep -s -n "Automatic changes above" cegcc.h.in | awk -F: '{print $1}'`
	head -$L1 cegcc.h.in >cegcc.h
	echo "#define   __CEGCC_VERSION_MAJOR__ " $CEGCC_VERSION_MAJOR >> cegcc.h
	echo "#define   __CEGCC_VERSION_MINOR__ " $CEGCC_VERSION_MINOR >> cegcc.h
	echo "#define   __CEGCC_VERSION_PATCHLEVEL__ " $CEGCC_VERSION_PATCHLEVEL >> cegcc.h
	echo "#define   __CEGCC_BUILD_DATE__" `date +%Y%m%d` >> cegcc.h
	tail +$L2 cegcc.h.in >>cegcc.h
	svn add cegcc.h
	svn commit cegcc.h -m "Version file for release $VERSION" && OK=yes
	if [ $OK = "no" ]; then
		echo "SVN commit failed, exiting..."
		exit 1
	fi
	#
	#	End level 21 (create version include file)
	#
	RESTARTED=30
	report_status
	#
fi

#
# Level 30
#
if test $RESTARTED -ge 31
then
	echo "Skip level 30 (modify README and script files)"
else
	OK=no
	cd $ORIGDIR
	cd $TAGSDIR/cegcc-$VERSION
	if [ -r README- ]; then
		echo "You appear to have tampered with the status file."
		echo "Do a better job at it : README- exists. Exiting."
		exit 1
	else
		mv README README-
		(head -2 README- ; \
		echo "This is version $VERSION of CeGCC."; \
		tail +4 README-) >README && OK=yes
	fi
	if [ $OK = "no" ]; then
		echo "Failed to edit README, exiting ..."
		exit 1
	fi
	cd scripts/linux && \
	rm -f cegcc.spec mingw32ce.spec rpm-create-source.sh && \
	sed -e "s/VerSION/$VERSION/g" <cegcc.spec.in >cegcc.spec && \
	sed -e "s/VerSION/$VERSION/g" <mingw32ce.spec.in >mingw32ce.spec && \
	sed -e "s/VerSION/$VERSION/g" <mingw32ce-i386.spec.in >mingw32ce-i386.spec && \
	sed -e "s/VerSION/$VERSION/g" <rpm-create-source.sh.in >rpm-create-source.sh && \
	OK=yes
	if [ $OK = "no" ]; then
		echo "Failed to do file editing, exiting"
		exit 1
	fi
	#
	#	End level 30 (modify README and script files)
	#
	RESTARTED=40
	report_status
	#
fi

#
# Level 40
#
if test $RESTARTED -ge 41
then
	echo "Skip level 40 (commit previous changes to SVN)"
else
	echo "Level 40 (commit previous changes to SVN)"
	OK=no
	cd $ORIGDIR
	cd $TAGSDIR/cegcc-$VERSION && \
	svn commit -m "Specific file versions for CeGCC $VERSION" \
		README \
		scripts/linux && \
	OK=yes
	if [ $OK = "no" ]; then
		echo "Failed to commit changes, exiting ..."
		exit 1
	fi
	#
	#	End level 40 (commit previous changes to SVN)
	#
	RESTARTED=50
	report_status
	#
fi

#
# Level 50
#
if test $RESTARTED -ge 51
then
	echo "Skip level 50 (Prepare for building RPMs)"
else
	echo "Level 50 (Prepare for building RPMs)"
	OK=no
	cd $ORIGDIR
	cd $TAGSDIR/cegcc-$VERSION/scripts/linux
	sh rpm-create-source.sh && OK=yes
	if [ $OK = "no" ]; then
		echo "Prepare RPM source tar.gz file failed, exiting ..."
		exit 1
	fi
	RESTARTED=51
	report_status
	#
fi
echo Stop
exit 0
#
# Level 51
#
if test $RESTARTED -ge 52
then
	echo "Skip level 51 (Build CeGCC RPM)"
else
	echo "Level 51 (Build CeGCC RPM)"
	OK=no
	rm -rf /opt/cegcc/* &&
	rpmbuild -tb /usr/src/rpm/SOURCES/cegcc-cegcc-src-$VERSION.tar.gz && OK=yes
	if [ $OK = "no" ]; then
		echo "Build of CeGCC RPM failed, exiting ..."
		exit 1
	fi
	if [ ! -r /usr/src/rpm/RPMS/cegcc-cegcc-$VERSION-1.i586.rpm ]; then
		echo "RPM doesn't appear to be there, exiting ..."
		echo "ls -l /usr/src/rpm/RPMS/i586/cegcc-cegcc-$VERSION-1.i586.rpm"
		exit 1
	fi
	#
	#	End level 51 (Build CeGCC RPM)
	#
	RESTARTED=52
	report_status
	#
fi
#
# Level 52
#
if test $RESTARTED -ge 53
then
	echo "Skip level 52 (Build Mingw32ce RPM)"
else
	echo "Level 52 (Build Mingw32ce RPM)"
	OK=no
	rm -rf /opt/mingw32ce/* &&
	rpmbuild -tb /usr/src/rpm/SOURCES/cegcc-mingw32ce-src-$VERSION.tar.gz && OK=yes
	if [ $OK = "no" ]; then
		echo "Build of Mingw32ce RPM failed, exiting ..."
		exit 1
	fi
	if [ ! -r /usr/src/rpm/RPMS/cegcc-mingw32ce-$VERSION-1.i586.rpm ]; then
		echo "RPM doesn't appear to be there, exiting ..."
		echo "ls -l /usr/src/rpm/RPMS/i586/cegcc-mingw32ce-$VERSION-1.i586.rpm"
		exit 1
	fi
	#
	#	End level 52 (Build Mingw32ce RPM)
	#
	RESTARTED=53
	report_status
	#
fi
#
# Level 53
#
if test $RESTARTED -ge 54
then
	echo "Skip level 53 (Build Mingw32ce/i386 RPM)"
else
	echo "Level 53 (Build Mingw32ce/i386 RPM)"
	OK=no
	rm -rf /opt/x86mingw32ce/* &&
	rpmbuild -tb /usr/src/rpm/SOURCES/cegcc-mingw32ce-i386-src-$VERSION.tar.gz && OK=yes
	if [ $OK = "no" ]; then
		echo "Build of Mingw32ce/i386 RPM failed, exiting ..."
		exit 1
	fi
	if [ ! -r /usr/src/rpm/RPMS/cegcc-mingw32ce-i386-$VERSION-1.i586.rpm ]; then
		echo "RPM doesn't appear to be there, exiting ..."
		echo "ls -l /usr/src/rpm/RPMS/i586/cegcc-mingw32ce-i386-$VERSION-1.i586.rpm"
		exit 1
	fi
	#
	#	End level 53 (Build Mingw32ce/i386 RPM)
	#
	RESTARTED=57
	report_status
	#
fi
#
# Level 57
#
if test $RESTARTED -ge 58
then
	echo "Skip level 57 (Build Mingw32ce gdb)"
else
	echo "Level 57 (Build Mingw32ce gdb)"
	OK=no
#	rm -rf /opt/x86mingw32ce/* &&
#	rpmbuild -tb /usr/src/rpm/SOURCES/cegcc-mingw32ce-i386-src-$VERSION.tar.gz && OK=yes
	if [ $OK = "no" ]; then
		echo "Build of Mingw32ce gdb failed, exiting ..."
		exit 1
	fi
	#
	#	End level 57 (Build Mingw32ce gdb)
	#
	RESTARTED=58
	report_status
	#
fi
#
# Level 58
#
if test $RESTARTED -ge 59
then
	echo "Skip level 58 (Build Mingw32ce/i386 gdb)"
else
	echo "Level 58 (Build Mingw32ce/i386 gdb)"
	OK=no
#	rm -rf /opt/x86mingw32ce/* &&
#	rpmbuild -tb /usr/src/rpm/SOURCES/cegcc-mingw32ce-i386-src-$VERSION.tar.gz && OK=yes
	if [ $OK = "no" ]; then
		echo "Build of Mingw32ce/i386 gdb failed, exiting ..."
		exit 1
	fi
	#
	#	End level 58 (Build Mingw32ce/i386 gdb)
	#
	RESTARTED=60
	report_status
	#
fi
#
# Level 60
#
if test $RESTARTED -ge 61
then
	echo "Skip level 60 (FTP RPM)"
else
	echo "Level 60 (FTP RPM)"
	#
	cd /usr/src/rpm/RPMS/i586
	FTPFILE=/tmp/lftp-$$
	echo "lcd /usr/src/rpm/RPMS/i586" >>$FTPFILE
	echo "cd uploads" >>$FTPFILE
	#
	# Cope with potentially different file names generated by rpmbuild.
	#
	LIST=`ls cegcc-cegcc-$VERSION-*.i586.rpm`
	COUNT=`echo $LIST | wc -w`
	if [ $COUNT -ne 1 ]; then
		echo "Unexpected number of RPM files, please clean up before continuing."
		exit 1
	fi
	if [ "x$LINUXDISTRO" = "x" ]; then
		PREFIX=""
	else
		PREFIX="$LINUXDISTRO-"
	fi
	echo "put $LIST -o $PREFIX$LIST" >>$FTPFILE
	LIST=`ls cegcc-mingw32ce-$VERSION-*.i586.rpm`
	COUNT=`echo $LIST | wc -w`
	if [ $COUNT -ne 1 ]; then
		echo "Unexpected number of RPM files, please clean up before continuing."
		exit 1
	fi
	echo "put $LIST -o $PREFIX$LIST" >>$FTPFILE
	echo "quit" >>$FTPFILE
	sftp -b $FTPFILE dannybackx,cegcc@frs.sourceforge.net
	#
	#	End level 60 (FTP RPM)
	#
	RESTARTED=70
	report_status
	#
fi

#
# Level 70
#
if test $RESTARTED -ge 71
then
	echo "Skip level 70 (Build binary tar images)"
else
	echo "Level 70 (Build binary tar images)"
	if [ "x$LINUXDISTRO" = "x" ]; then
		PREFIX=""
	else
		PREFIX="$LINUXDISTRO-"
	fi
	OK="no"
	tar cfz /tmp/${PREFIX}cegcc-cegcc-$VERSION.tar.gz /opt/cegcc && \
	tar cfz /tmp/${PREFIX}cegcc-mingw32ce-$VERSION.tar.gz /opt/mingw32ce && \
	OK="yes"
	if [ $OK = "no" ]; then
		echo "Creating binary tar failed, exiting ..."
		exit 1
	fi
	#
	#	End level 70 (Build binary tar images)
	#
	RESTARTED=80
	report_status
	#
fi

#
# Level 80
#
if test $RESTARTED -ge 81
then
	echo "Skip level 80 (FTP tar images)"
else
	echo "Level 80 (FTP tar images)"
	FTPFILE=/tmp/lftp-$$
	echo "lcd /tmp" >>$FTPFILE
	echo "cd uploads" >>$FTPFILE
	#
	if [ "x$LINUXDISTRO" = "x" ]; then
		PREFIX=""
	else
		PREFIX="$LINUXDISTRO-"
	fi
	echo "put ${PREFIX}cegcc-cegcc-$VERSION.tar.gz" >>$FTPFILE
	echo "put ${PREFIX}cegcc-mingw32ce-$VERSION.tar.gz" >>$FTPFILE
	echo "lcd /usr/src/rpm/SOURCES" >>$FTPFILE
#
# These are a bit big, commenting out.
#
#	echo "put cegcc-cegcc-src-$VERSION.tar.gz" >>$FTPFILE
#	echo "put cegcc-mingw32ce-src-$VERSION.tar.gz" >>$FTPFILE
	echo "quit" >>$FTPFILE
	OK=no
	sftp -b $FTPFILE dannybackx,cegcc@frs.sourceforge.net && OK=yes
	if [ $OK = "no" ]; then
		echo "FTP binary tar failed, exiting..."
		exit 1
	fi
	#
	#	End level 80 (FTP binary tar images)
	#
	RESTARTED=90
	report_status
	#
fi

#
# Level 90
#
if test $RESTARTED -ge 91
then
	echo "Skip level 90 ()"
else
	#
	#	End level 90 ()
	#
	RESTARTED=100
	report_status
	#
fi

#
# Report progress
#
report_status
#
#
echo " "
echo "Exiting $0, result status $RESTARTED."
echo " "
exit 0
