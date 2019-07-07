include ../../smax-src/common.mk

maxsat-test-dynamic: maxsat-test.cc Makefile
	g++ maxsat-test.cc -I../.. -L../../lib -lsmax -std=c++11 -lz -lgmp -o maxsat-test-dynamic -O0 -g $(CFLAGS) $(EXTRA_CFLAGS)

maxsat-test: maxsat-test.cc Makefile
	g++ maxsat-test.cc -I../.. -L../../lib -lsmax -std=c++11 -lz -lgmp -o maxsat-test -static -O0 -g $(CFLAGS) $(EXTRA_CFLAGS)

clean:
	rm -f maxsat-test maxsat-test-dynamic
