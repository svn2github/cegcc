After having binutils, gcc and newlib installed
run make here to produce cegcc.dll and the corresponding
import lib libcegcc.dll.a.

type:
 make install to install to $(PREFIX), usually /opt/cegcc

type:
 make installdll to copy stripped dll to \Windows\ on device
 (depends on itsutils being present)
