CmdlineGL
=========

CmdlineGL is an interpreter for a "text-friendly" variation of a subset of the
OpenGL 1.4 API, Glut API, and FTGL "C" API.

It reads API calls on stdin, and writes events on stdout, while reporting
errors to stderr.  You can find a description of the available commands and
events in the manual page, and get a list of the functions and constants
supported by a particular build of CmdlineGL using the
``--showcmds`` and ``--showconstants`` options.

Dependencies
============

Before you can build CmdlineGL, you need: SDL, SDL_image, OpenGL, GLU, FTGL,
and the development headers for each.  On Debian/Ubuntu/Mint, these are:

    sudo apt-get install \
        libsdl1.2-dev libsdl-image1.2-dev \
        libgl1-mesa-dev libftgl-dev

and on Fedora/RHEL they are

    sudo yum install \
        SDL-devel SDL_image-devel \
        mesa-libGL-devel mesa-libGLU-devel ftgl-devel

The autoconf script is still very new, so it might not detect locations of
headers and libraries on other systems.  Patches are welcome.

Building and Installing
=======================

This is almost but not quite a standard autoconf distribution.  All the
autoconf files live in the ./script directory, and the top level project has
a pass-through Makefile and ./configure script that set up an out-of-tree
(well, technically still in the project tree) build in a directory named
"./build".  Anyway, you can just run

    make && sudo make install

from the root of the project and maybe everything will just work.  If you are
running from the distribution tarball, this will create a "production build",
but if you are running it from a git checkout, it will create a "dev build"
with assertions enabled and some extra logging.

You can also invoke autoconf directly:

    mkdir work
    cd work
    $PROJDIR/script/configure [OPTIONS]
    make
    make install

Usage
=====

See the manual page (built during 'make' process) for details about the commands
supported. There is also the [online manual], and [examples directory].

[examples directory]: ./share/examples
[online manual]: https://www.nrdvana.net/cmdlinegl/release/current/CmdlineGL.html
