After having binutils/gcc/newlib installed
run make here to produce cegcc.dll and the corresponding
import lib libcegcc.dll.a.

type:
 make install to install to $(PREFIX), usually /usr/local

type:
 make installdll to copy stripped dll to \Windows\ on device
 (depends on itsutils being present)