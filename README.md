CmdlineGL
=========

CmdlineGL is an interpreter for a "text-friendly" variation of a subset of the OpenGL 1.4 API,
Glut API, and FTGL "C" API.

It reads API calls on stdin, and writes events on stdout, while reporting errors to stderr.
You can find a description of the available commands and events in the manual page, and get a
list of the functions and constants supported by a particular build of CmdlineGL using the
``--showcmds`` and ``--showconstants`` options.

Dependencies
============

Before you can build CmdlineGL, you need: SDL, SDL_image, OpenGL, GLU, FTGL, and the
development headers for each.  On Debian/Ubuntu/Mint, these are:

  sudo apt-get install libsdl1.2-dev libsdl-image1.2-dev libgl1-mesa-dev libftgl-dev

and on Fedora/RHEL they are

  sudo yum install SDL-devel mesa-libGL-devel mesa-libGLU-devel SDL_image-devel ftgl-devel

