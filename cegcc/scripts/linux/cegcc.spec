Summary:	CeGCC offers cross-development to create Windows CE apps for ARM processors
Name:		cegcc
Version:	0.11
Release:	1
License:	open
Packager:	Danny Backx <dannybackx@users.sourceforge.net>
Group:		Development/Tools
Prefixes:	/opt
# Source:		http://sourceforge.net/project/showfiles.php?group_id=173455
Source:		/tmp/cegcc-src-0.12.tar.gz
# BuildRoot:	/tmp/cegcc

%description
CeGCC

#
# In the scripts run from "rpmbuild -ba cegcc.spec", we appear to have
# access to these variables :
#	RPM_OPT_FLAGS=-O2  ...
#	RPM_PACKAGE_RELEASE=0.06
#	RPM_PACKAGE_NAME=CeGCC
#	RPM_SOURCE_DIR=/usr/src/RPM/SOURCES
#	RPM_PACKAGE_VERSION=0.06
#	RPM_OS=linux
#	RPM_DOC_DIR=/usr/share/doc
#	RPM_BUILD_DIR=/usr/src/RPM/BUILD
#	RPM_ARCH=i386
#	
%prep
%setup -n cegcc-0.12

echo script

%build
export MY_RPM_PREFIX=/tmp/rpm-$RPM_PACKAGE_NAME-$RPM_PACKAGE_RELEASE
cd src || exit 1
sh build-cegcc.sh all || exit 1
sh build-mingw32ce.sh all || exit 1

%install
# Nothing to do

%files
/opt/cegcc/arm-wince-cegcc
/opt/mingw32ce/arm-wince-mingw32ce
%attr(755, root, root) /opt/cegcc/bin
/opt/cegcc/include
/opt/cegcc/lib
/opt/cegcc/libexec
/opt/cegcc/share
/opt/cegcc/COPYING
/opt/cegcc/COPYING.LIB
/opt/cegcc/NEWS
/opt/cegcc/README
%attr(755, root, root) /opt/mingw32ce/bin
/opt/mingw32ce/include
/opt/mingw32ce/lib
/opt/mingw32ce/libexec
/opt/mingw32ce/share
/opt/mingw32ce/COPYING
/opt/mingw32ce/COPYING.LIB
/opt/mingw32ce/NEWS
/opt/mingw32ce/README

%changelog
* Sat Dec 30 2006 Danny Backx <dannybackx@users.sf.net>
- Adapt to Pedro's build scripts and /opt/cegcc and /opt/mingw32ce .

* Wed Nov 1 2006 Danny Backx <dannybackx@users.sf.net>
- Add COPYING.LIB
- Increase level to produce a 0.11 version next time.

* Wed Oct 11 2006 Danny Backx <dannybackx@users.sf.net>
- Add a couple of text files.

* Sat Oct 7 2006 Danny Backx <dannybackx@users.sf.net>
- Change to implement arm-wince-cegcc and arm-wince-mingw32ce targets.

* Sun Sep 17 2006 Danny Backx <dannybackx@users.sf.net>
- Add documentation files.

* Thu Sep 14 2006 Danny Backx <dannybackx@users.sf.net>
- Fix the path in install so we don't need to have cegcc installed to be
  able to run rpmbuild.

* Sun Sep 3 2006 Danny Backx <dannybackx@users.sf.net>
- initial version of the spec file.
