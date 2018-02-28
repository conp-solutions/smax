all: libs smax

libs:
	cd smax-src; make libl
	mkdir -p lib
	cp smax-src/libsmax_standard.so lib/libsmax.so
	cd smax-src; make libd
	cp smax-src/libsmax_debug.a lib/libsmax.a

smax:
	cd smax-src; make rs

clean:
	cd smax-src; make clean
	rm -f lib/libsmax.a lib/libsmax.so
