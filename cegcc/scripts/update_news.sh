#!/bin/sh
cd $HOME_DIR/tmp
ATOM=atom.xml
HTML=news_table.html

mv -f ${ATOM} ${ATOM}.prev
mv -f ${HTML} ${HTML}.prev
wget http://cegcc.blogspot.com/atom.xml -O ${ATOM}
php -f $NIGHTLY_DIR/scripts/blogger.php > $WEB_DIR/${HTML}
