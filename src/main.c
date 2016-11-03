/*
 * Ubivat Tank
 * Главное тело
 * by Master San
 */

#include <types.h>

#include <game.h>
#include <img.h>
#include <map.h>
#include <weap.h>
#include <menu.h>
#include <_gr2D.h>
#include <fonts.h>
#include <player.h>

#include <stdio.h>
#include <string.h>
#include <think.h>

//#include <fonts.h>

int main()
{
//	fonts_test();

	game_init();
	game_main();
	game_done();
	return 0;
}

