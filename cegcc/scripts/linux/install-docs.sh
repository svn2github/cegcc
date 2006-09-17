#!/bin/sh
#
# Read the settings
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi
cd $TOP_SRCDIR || exit 1
export DOCDIR=${PREFIX}/share/doc/cegcc-${CEGCC_RELEASE} || exit 1
mkdir -p ${DOCDIR} ${DOCDIR}/images ${DOCDIR}/docs ${DOCDIR}/docs/examples || exit 1
#
cp docs/*.html ${DOCDIR}/docs || exit 1
cp docs/examples/Makefile ${DOCDIR}/docs/examples || exit 1
cp docs/examples/*.{c,exe,h,rc,tar.gz} ${DOCDIR}/docs/examples || exit 1
cp website/*.html ${DOCDIR} || exit 1
cp website/images/*.png ${DOCDIR}/images || exit 1
#
exit 0
