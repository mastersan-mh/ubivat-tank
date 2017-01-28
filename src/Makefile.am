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
common_bstree.c \
common_hash.c \
Z_mem.c \
system.c \
net.c \
video.c \
sound.c \
utf8.c \
types.c \
_gr2D.c \
fonts.c \
actions.c \
game.c \
g_conf.c \
g_gamesave.c \
img.c \
main.c \
map.c \
entity.c \
entity_helpers.c \
model.c \
model_resources.c \
ui.c \
menu.c \
server.c \
sv_game.c \
client.c \
cl_input.c \
cl_game.c
