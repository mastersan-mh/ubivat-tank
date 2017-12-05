MAINTAINERCLEANFILES = Makefile.in

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = ubivat-tank

ubivat_tank_CFLAGS = -std=c99 -Wall -D_DEFAULT_SOURCE -I/usr/include/freetype2
ubivat_tank_LDFLAGS = -lm -lGL -lGLU -lSDL2 -lSDL2_image -lfreetype -lfontconfig

ubivat_tank_SOURCES = \
common/common_hash.c \
progs/progs_main.c \
progs/helpers.c \
progs/ent_exit.c \
progs/ent_message.c \
progs/ent_spawn.c \
progs/ent_bull.c \
progs/ent_explode.c \
progs/ent_weap.c \
progs/ent_items.c \
progs/ent_player.c \
progs/ent_player_think.c \
Z_mem.c \
vars.c \
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
game_progs.c \
game_progs_internal.c \
g_conf.c \
g_gamesave.c \
img.c \
main.c \
map.c \
world.c \
entity_internal.c \
entity.c \
model.c \
model_resources.c \
ui.c \
menu.c \
server_private.c \
server_fsm.c \
server_events.c \
server_reply_private.c \
sv_game.c \
server.c \
client_private.c \
client_fsm.c \
client_events.c \
client_requests.c \
cl_game.c \
cl_input.c \
client.c
