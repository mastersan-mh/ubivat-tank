AC_PREREQ(2.69)
# AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_INIT([ubivat-tank], [1.0], [mastersan_mh@list.ru])
AM_CONFIG_HEADER(src/config.h)
AC_CONFIG_SRCDIR(src/main.c)

AM_INIT_AUTOMAKE([1.13 foreign no-define subdir-objects])

# Checks compiler
AC_PROG_CC

AC_CONFIG_FILES([
Makefile
src/Makefile
])

AC_CHECK_LIB([m], [cosf], [], [ AC_MSG_ERROR([Unable to find 'm']) ] )
AC_CHECK_LIB([GL], [glBegin], [], [ AC_MSG_ERROR([Unable to find 'GL']) ] )
AC_CHECK_LIB([GLU], [gluOrtho2D], [], [ AC_MSG_ERROR([Unable to find 'GLU']) ] )
AC_CHECK_LIB([SDL2], [SDL_GetVersion], [], [ AC_MSG_ERROR([Unable to find 'SDL2']) ] )
AC_CHECK_LIB([freetype], [FT_Init_FreeType], [], [ AC_MSG_ERROR([Unable to find 'freetype']) ] )

AC_OUTPUT
