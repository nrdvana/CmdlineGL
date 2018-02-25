# Process this file with autoconf to produce a configure script.
#
AC_PREREQ(2.59)
AC_INIT(CmdlineGL, 1.0, mike@nrdvana.net)
AC_CONFIG_SRCDIR([Global.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LN_S
AC_PROG_MAKE_SET
#AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB(GL,glBegin)
AC_CHECK_LIB(GLU,gluNewQuadric)
AC_CHECK_LIB(ftgl,ftglCreateTextureFont)
AC_CHECK_LIB(SDL,SDL_Init)
AC_CHECK_LIB(SDL_image,IMG_Load)

# Checks for header files.
AC_PATH_X
if test "X${x_includes}" != "X"; then
	CFLAGS="${CFLAGS} -I${x_includes}"
fi
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h])
AC_CHECK_HEADERS([GL/gl.h GL/glu.h],[AC_SUBST(gl_header,GL/gl.h) AC_SUBST(glu_header,GL/glu.h)],[AC_CHECK_HEADERS([OpenGL/gl.h OpenGL/glu.h],[AC_SUBST(gl_header,OpenGL/gl.h) AC_SUBST(glu_header,OpenGL/glu.h)],[AC_MSG_ERROR([Can't find OpenGL headers])])])
AC_CHECK_HEADERS([SDL/SDL.h],[AC_SUBST(sdl_header,SDL/SDL.h)],[AC_MSG_ERROR([Can't find SDL header (apt-get install libsdl1.2-dev)])])
AC_CHECK_HEADERS([SDL/SDL_image.h],[AC_SUBST(sdl_image_header,SDL/SDL_image.h)],[AC_MSG_ERROR([Can't find SDL_image header (apt-get install libsdl-image1.2-dev)])])
CFLAGS="${CFLAGS} -I/usr/include/freetype2"
CPPFLAGS="${CPPFLAGS} -I/usr/include/freetype2"
AC_CHECK_HEADERS([FTGL/ftgl.h],[AC_SUBST(ftgl_header,FTGL/ftgl.h)],[AC_MSG_ERROR([Can't find FTGL header (apt-get install libftgl-dev)])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([memmove mkfifo strdup strtol atexit memset])

AC_CONFIG_FILES([Makefile GlHeaders.h SDLHeaders.h])
AC_OUTPUT
