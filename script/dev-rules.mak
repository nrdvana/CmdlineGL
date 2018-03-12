# These rules are only needed for authoring CmdlineGL and don't need to be
# active in the release tarball.  I wanted the auto-generated C files to
# go here as well, but the hash tables depend on the available libraries
# on the host system, so can't be generated in advance.

$(scriptdir)/config-defs.h.in: $(scriptdir)/configure.ac
	cd $(scriptdir) && autoheader && touch $(scriptdir)/config-defs.h.in

$(scriptdir)/configure: $(scriptdir)/configure.ac
	cd $(scriptdir) && autoconf

autogen_files: $(scriptdir)/config-defs.h.in $(scriptdir)/configure

dist: $(srcdir)/../.git
	env PROJROOT=$(srcdir)/.. $(PERL) $(scriptdir)/build-dist.pl

.PHONY: autogen_files dist
