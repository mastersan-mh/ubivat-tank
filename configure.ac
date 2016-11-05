AC_PREREQ(2.69)
# AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_INIT([ubivat-tank], [0.47b], [mastersan_mh@list.ru])
AM_CONFIG_HEADER(src/config.h)
AC_CONFIG_SRCDIR(src/main.c)

AM_INIT_AUTOMAKE(1.13 foreign no-define)

# Checks compiler
AC_PROG_CC

AC_CHECK_HEADER([math.h], [], [ AC_MSG_ERROR([Unable to find math.h]) ] )
AC_CHECK_LIB([m], [fabs], [], [ AC_MSG_ERROR([Unable to find libm])   ] )

AC_CHECK_HEADER([SDL2/SDL.h], [], [],
[[#ifdef HAVE_SDL2_SDL_H
# include <SDL2/SDL.h>
#endif
]]
)
AC_CHECK_LIB([SDL2], [SDL_Init], [], [ AC_MSG_ERROR([Unable to find libSDL2])    ] )

AC_CHECK_HEADER([GL/gl.h], [], [ AC_MSG_ERROR([Unable to find GL/gl.h]) ] )
AC_CHECK_LIB([GL], [glEnable], [])

AC_CHECK_HEADER([GL/glu.h],     [], [ AC_MSG_ERROR([Unable to find GL/glu.h]) ] )
AC_CHECK_LIB([GLU], [gluOrtho2D])

AC_CHECK_HEADER([fontconfig/fontconfig.h], [], [ AC_MSG_ERROR([Unable to find fontconfig/fontconfig.h]) ] )
AC_CHECK_LIB([fontconfig], [FcUtf8ToUcs4])

#AC_CHECK_HEADER([freetype2/freetype.h], [], [ AC_MSG_ERROR([Unable to find freetype2/freetype.h]) ] )

AC_CHECK_HEADERS([freetype2/config/ftheader.h], [], [ AC_MSG_ERROR([Unable to find freetype2/config/ftheader.h]) ])


AC_CHECK_HEADERS([freetype2/freetype.h])

#[[#ifdef HAVE_FREETYPE2_FREETYPE_H
##	include <freetype2/freetype.h>
##endif
#]]
#)

#AC_CHECK_HEADER([freetype2/ft2build.h], [], [ AC_MSG_ERROR([Unable to find freetype2/ft2build.h]) ] )
AC_CHECK_HEADER([freetype2/ft2build.h], [], [],
[[#ifdef HAVE_FREETYPE2_Ft2BUILD_H
#	include <freetype2/ft2build.h>
#endif
]]
)
AC_CHECK_LIB([freetype], [FT_Init_FreeType])


#AC_ARG_WITH(with-basedir,AC_HELP_STRING([--with-basedir],[Path to install base directory]),
#UTANKBASEDIR="$withval",
#if test "x$prefix" != xNONE; then
#  UTANKBASEDIR="$prefix/share/games/ubivat-tank"
#else
#  UTANKBASEDIR="$ac_default_prefix/share/games/ubivat-tank"
#fi
#)
#AC_DEFINE_UNQUOTED(UTANKBASEDIR,"$UTANKBASEDIR",[Path to install base directory])

if test "x$prefix" != xNONE; then
  UTANKBASEDIR="$prefix/share/games/ubivat-tank"
else
  UTANKBASEDIR="$ac_default_prefix/share/games/ubivat-tank"
fi

AC_DEFINE_UNQUOTED(UTANKBASEDIR,"$UTANKBASEDIR",[Path to install base directory])

AC_CONFIG_FILES([
Makefile
src/Makefile
])
AC_OUTPUT
