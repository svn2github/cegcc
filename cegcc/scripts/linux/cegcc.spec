Summary:	CeGCC offers cross-development to create Windows CE apps for ARM processors
Name:		cegcc
Version:	0.08
Release:	2
License:	open
Packager:	Danny Backx <dannybackx@users.sourceforge.net>
Group:		Development/Tools
Prefixes:	/usr/ppc
# Source:		http://sourceforge.net/project/showfiles.php?group_id=173455
Source:		/tmp/cegcc-src-0.08.tar.gz
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
%setup -n cegcc-0.08

echo script

%build
export MY_RPM_PREFIX=/tmp/rpm-$RPM_PACKAGE_NAME-$RPM_PACKAGE_RELEASE
sh scripts/linux/rpm-build.sh

%install
#
# No special prefix here
#
export PATH=/tmp/rpm-$RPM_PACKAGE_NAME-$RPM_PACKAGE_RELEASE/bin:$PATH
sh scripts/linux/rpm-install.sh

%files
/usr/ppc/arm-wince-pe
/usr/ppc/arm-wince-mingw32
%attr(755, root, root) /usr/ppc/bin
/usr/ppc/include
/usr/ppc/info
/usr/ppc/lib
/usr/ppc/libexec
/usr/ppc/man
# /usr/ppc/NEWS
/usr/ppc/share

%changelog
* Sun Sep 17 2006 Danny Backx <dannybackx@users.sf.net>
- Add documentation files.

* Thu Sep 14 2006 Danny Backx <dannybackx@users.sf.net>
- Fix the path in install so we don't need to have cegcc installed to be
  able to run rpmbuild.

* Sun Sep 3 2006 Danny Backx <dannybackx@users.sf.net>
- initial version of the spec file.
