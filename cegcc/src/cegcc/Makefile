all: cegccdll cegccthrd libstdc++

cegccdll: 
	cd cegccdll; \
	make; \
	make install
	
cegccthrd:
	cd cegccthrd; \
	make; \
	make install

libstdc++:
	cd libstdc++; \
	make; \
	make install

install:
	cd cegccdll; make installdll;
	cd cegccthrd; make installdev;
	cd libstdc++; make installdll;

clean:
	cd cegccdll; make clean;
	cd cegccthrd; make clean;
	cd libstdc++; make clean;

.PHONY: cegccdll cegccthrd libstdc++ install clean
