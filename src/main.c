/*
 * Ubivat Tank
 * Главное тело
 * by Master San
 */

#include <defs.h>
#include <game.h>
#include <img.h>
#include <map.h>
#include <plr.h>
#include <weap.h>
#include <menu.h>
#include <ctrl.h>

#include <_gr2D.h>
#include <_gr2Don.h>
#include <x10_str.h>
#include <x10_kbrd.h>
#include <x10_time.h>

#include <stdio.h>
#include <string.h>

int main()
{
	game_init();
	game_main();
	game_done();
	return 0;
}

