MAINTAINERCLEANFILES = Makefile.in

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = ubivat-tank
SUBDIRS = 

ubivat_tank_CFLAGS = -std=c99 -Wall -D_DEFAULT_SOURCE -I/usr/include/freetype2
ubivat_tank_LDFLAGS = -lm -lSDL2 -lGL -lGLU -lfreetype

ubivat_tank_SOURCES = \
Z_mem.c \
system.c \
types.c \
video.c \
_gr2D.c \
_gr2Don.c \
actions.c \
input.c \
think.c \
game.c \
img.c \
main.c \
map.c \
menu.c \
player.c \
weap.c
