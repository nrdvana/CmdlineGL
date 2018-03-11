AC_PREREQ([2.68])
AC_INIT([CmdlineGL])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config-defs.h])

AC_ARG_ENABLE(debug,  AS_HELP_STRING([debug], [enable assertions and debug symbols]),
 [ if test "$enableval" != "no"; then CFLAGS="$CFLAGS -O0 -g3"; else CFLAGS="$CFLAGS -O2 -DNDEBUG"; fi; ],
 [ CFLAGS="$CFLAGS -O0 -g3"; ])

# AC_ARG_ENABLE(dev,    AS_HELP_STRING([dev], [enable source generators (requires perl)]),
# [ if test "$enableval" != "no"; then dev_include_makefile="\$(scriptdir)/dev_rules.mak"; fi; ],
# [ dev_include_makefile="\$(scriptdir)/dev_rules.mak"; ])
# AC_SUBST(dev_include_makefile)

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB(GL,glBegin)
AC_CHECK_LIB(GLU,gluNewQuadric)
AC_CHECK_LIB(ftgl,ftglCreateTextureFont)
LDFLAGS="${LDFLAGS} `sdl-config --libs`"
AC_CHECK_LIB(SDL,SDL_Init)
AC_CHECK_LIB(SDL_image,IMG_Load)

# Checks for header files.
AC_PATH_X
if test "X${x_includes}" != "X"; then
	CFLAGS="${CFLAGS} -I${x_includes}"
fi
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h stdbool.h stdint.h])
AC_CHECK_HEADERS([GL/gl.h GL/glu.h],[AC_SUBST(gl_header,GL/gl.h) AC_SUBST(glu_header,GL/glu.h)],[AC_CHECK_HEADERS([OpenGL/gl.h OpenGL/glu.h],[AC_SUBST(gl_header,OpenGL/gl.h) AC_SUBST(glu_header,OpenGL/glu.h)],[AC_MSG_ERROR([Can't find OpenGL headers])])])
CFLAGS="${CFLAGS} `sdl-config --cflags`"
AC_CHECK_HEADERS([SDL/SDL.h],[AC_SUBST(sdl_header,SDL/SDL.h)],[AC_MSG_ERROR([Can't find SDL header (apt-get install libsdl1.2-dev)])])
AC_CHECK_HEADERS([SDL/SDL_image.h],[AC_SUBST(sdl_image_header,SDL/SDL_image.h)],[AC_MSG_ERROR([Can't find SDL_image header (apt-get install libsdl-image1.2-dev)])])
CFLAGS="${CFLAGS} -I/usr/include/freetype2"
CPPFLAGS="${CPPFLAGS} -I/usr/include/freetype2"
AC_CHECK_HEADERS([FTGL/ftgl.h],[AC_SUBST(ftgl_header,FTGL/ftgl.h)],[AC_MSG_ERROR([Can't find FTGL header (apt-get install libftgl-dev)])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([snprintf])

AC_CONFIG_FILES([Makefile config.h])
AC_OUTPUT
