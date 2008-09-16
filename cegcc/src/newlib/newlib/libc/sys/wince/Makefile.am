## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

net_SOURCES = ascii2addr.c htons.c htonl.c inet_aton.c inet_lnaof.c inet_mkadr.c \
			  inet_net.c inet_netof.c inet_ntoa.c msnet.c wsdb.c rexec.c wsstrerror.c
unix_SOURCES = getcwd.c getlogin.c sleep.c usleep.c vfork.c realpath.c
celib_SOURCES = cecopyfile.c cefileattr.c cefindfile.c cefixpath.c ceirda.c ceutil.c cethread.c cemakeunixpath.c \
				cemovefile.c cecreatefile.c cecreatefilemap.c ceregistry.c cecurrentdir.c ceprofile.c \
				ceshared2.c ceremovedir.c cemisc.c
				
mingw_SOURCES = _crt_mt.c mthr_thunk.c mthr_ptrs.c pseudo-reloc.c pseudo-reloc_t.c

lib_a_SOURCES = __eh_continue.S abort.c alloc.c assert.c ceprocess.c cs.c dirent.c dllmain.c \
				dllmaincrtstartup.c dllmaincrtstartup_t.c env.c error.c fifo.c findpath.c fixpath.c fnmatch.c \
				getopt.c getopt1.c getreent.c glob.c group.c ipfcns.c io.c issetugid-stub.c llst.c malloc.c \
				mqueue.c pathconf.c popen.c sclass.c shared.c sig.c sigimp.c spawn.c \
				startup.c startup_thunk.c stat.c system.c termios.c timefcns.c trace.c tzset_hook_r.c \
				uid.c unistd.c utime.c main.c \
				$(net_SOURCES) \
				$(unix_SOURCES) \
				$(celib_SOURCES) \
				$(mingw_SOURCES)

all: crt0.o dllcrt1.o crtst.o crtmt.o

crt0.o: crt0.S
dllcrt1.o: dllcrt1.S
crtst.o: crtst.c
crtmt.o: crtmt.c

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/arpa; \
	for i in $(srcdir)/arpa/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/arpa/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/asm; \
	for i in $(srcdir)/asm/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/asm/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/netinet; \
	for i in $(srcdir)/netinet/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/netinet/`basename $$i`; \
	done;

install-exec-hook:	
	$(INSTALL_PROGRAM) dllcrt1.o $(DESTDIR)$(tooldir)/lib/dllcrt1.o; \
	$(INSTALL_PROGRAM) crtmt.o $(DESTDIR)$(tooldir)/lib/crtmt.o; \
	$(INSTALL_PROGRAM) crtst.o $(DESTDIR)$(tooldir)/lib/crtst.o;

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
