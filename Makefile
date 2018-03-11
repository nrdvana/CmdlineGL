MAKE = make
PROVE = prove
PERL = perl

all: build/Makefile
	$(MAKE) -C build all

build/Makefile build/config.h: build/config.status script/Makefile.in script/config.h.in
	cd build && ./config.status

build/config.status: script/configure script/config-defs.h.in
	./configure

script/config-defs.h.in: script/configure.ac
	cd script && autoheader

script/configure: script/configure.ac
	cd script && autoconf

clean:
	$(MAKE) -C build clean

dist:
	env PROJ_ROOT=. $(PERL) script/build_dist_tarball.pl

test:
	$(MAKE) -C build daemonproxy
	$(PROVE) -j4

install:
	$(MAKE) -C build install

.PHONY: test all clean dist install
