/*
 * Ubivat Tank
 * Главное тело
 * by Master San
 */

#include "game.h"

//#include <fonts.h>

int main(int argc, char ** argv)
{
    //	fonts_test();

    game_init();
    game_main();
    game_done();
    return 0;
}
