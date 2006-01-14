# Process this file with autoconf to produce a configure script.
#
# I hereby declare that I have no clue what I'm doing when it comes
# to autoconf.  All suggestions (and patches!) are welcome.
#
AC_PREREQ(2.59)
AC_INIT(CmdlineGL, 0.9, silverdirk@silverdirk.com)
AC_CONFIG_SRCDIR([Global.c])
AC_CONFIG_HEADER([config.h])

AC_ARG_VAR(INST_SHELL_BIND,[Set to 'y' to install a script symlink into bin/ for each glXxxx command, so that they can be run from the command line])

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
AC_CHECK_LIB(SDL,SDL_Init)

# Checks for header files.
AC_PATH_X
if test "X${x_includes}" != "X"; then
	CFLAGS="${CFLAGS} -I${x_includes}"
fi
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h])
AC_CHECK_HEADERS([GL/gl.h GL/glu.h],[AC_SUBST(gl_header,GL/gl.h) AC_SUBST(glu_header,GL/glu.h)],[AC_CHECK_HEADERS([OpenGL/gl.h OpenGL/glu.h],[AC_SUBST(gl_header,OpenGL/gl.h) AC_SUBST(glu_header,OpenGL/glu.h)],[AC_MSG_ERROR([Can't find OpenGL headers])])])
AC_CHECK_HEADERS([SDL/SDL.h],[AC_SUBST(sdl_header,SDL/SDL.h)],[AC_MSG_ERROR([Can't find Glut header (is it installed?)])])

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

AC_CONFIG_FILES([Makefile GlHeaders.h])
AC_OUTPUT
