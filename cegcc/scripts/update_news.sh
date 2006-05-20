#!/bin/sh
cd $HOME_DIR/tmp
ATOM=atom.xml
HTML=news_table.html

mv -f ${ATOM} ${ATOM}.prev
# mv -f ${HTML} ${HTML}.prev
wget http://cegcc.blogspot.com/atom.xml -O ${ATOM}
php -f $NIGHTLY_DIR/scripts/blogger.php > ${HTML}

#
# CUT the index file in two pieces, put the new stuff in between.
#
LN=`grep -n "CUT HERE" $WEB_DIR/index.html | awk -F: '{print $1;}'`
head -$LN $WEB_DIR/index.html >ix1
tail +$LN $WEB_DIR/index.html >ix2
cat ix1 ${HTML} ix2 >$WEB_DIR/index.html
