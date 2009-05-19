SUBDIRS=	libstdc++ libsupc++


all::
	for i in ${SUBDIRS}; do \
		cd $$i ; \
	       	make all || exit 1; \
		cd .. ; \
		done


clean::
	for i in ${SUBDIRS}; do \
		cd $$i ; \
	       	make clean ; \
		cd .. ; \
		done

install::
	for i in ${SUBDIRS}; do \
		cd $$i ; \
	       	make install || exit 1; \
		cd .. ; \
		done

