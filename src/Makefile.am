MAINTAINERCLEANFILES = Makefile.in

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = ubivat-tank
SUBDIRS = 

ubivat_tank_CFLAGS = -std=c99 -Wall -D_DEFAULT_SOURCE -I/usr/include/freetype2
ubivat_tank_LDFLAGS = -lm -lGL -lGLU -lSDL2 -lSDL2_image -lfreetype -lfontconfig

ubivat_tank_SOURCES = \
Z_mem.c \
system.c \
video.c \
sound.c \
utf8.c \
types.c \
_gr2D.c \
fonts.c \
actions.c \
input.c \
think.c \
game.c \
img.c \
main.c \
map.c \
mobjs.c \
bull.c \
explode.c \
menu.c \
player.c \
client.c \
weap.c
