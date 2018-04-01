MAKE = make
PROVE = prove
PERL = perl

all: build
	$(MAKE) -C build all

clean: build
	$(MAKE) -C build clean

install: build
	$(MAKE) -C build install

build:
	[ -f script/configure ] || { cd script; autoheader; autoconf; }
	[ -d build ] || ./configure

.PHONY: test all clean dist install
