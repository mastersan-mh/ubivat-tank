MAINTAINERCLEANFILES = Makefile.in

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = ubivat-tank
SUBDIRS = 

ubivat_tank_CFLAGS = -std=c99 -Wall -D_DEFAULT_SOURCE -I/usr/include/freetype2
ubivat_tank_LDFLAGS = -lm -lGL -lGLU -lSDL2 -lSDL2_image -lfreetype -lfontconfig

ubivat_tank_SOURCES = \
ent_exit.c \
ent_message.c \
ent_spawn.c \
ent_bull.c \
ent_explode.c \
ent_weap.c \
ent_items.c \
ent_player.c \
ent_player_think.c \
Z_mem.c \
system.c \
video.c \
sound.c \
utf8.c \
types.c \
_gr2D.c \
fonts.c \
actions.c \
cl_input.c \
game.c \
img.c \
main.c \
map.c \
entity.c \
model.c \
model_resources.c \
menu.c \
server.c \
client.c
