MAKE = make
PROVE = prove
PERL = perl

all:
	[ -f script/configure ] || { cd script; autoheader; autoconf; }
	[ -d build ] || ./configure
	$(MAKE) -C build all

clean:
	$(MAKE) -C build clean

install:
	$(MAKE) -C build install

.PHONY: test all clean dist install
