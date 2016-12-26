
#include "system.h"

#include "game.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


#include <fontconfig/fontconfig.h>


#define TICRATE 1

static unsigned long lasttimereply;
static unsigned long basetime;

unsigned long system_getTime_realTime()
{
	struct timeval tv;
	unsigned long thistimereply;

	gettimeofday(&tv, NULL);

	thistimereply = (tv.tv_sec * TICRATE + (tv.tv_usec * TICRATE) / 1000000);

	/* Fix for time problem */
	if (!basetime) {
		basetime = thistimereply;
		thistimereply = 0;
	}
	else
	{
		thistimereply -= basetime;
	}
	if (thistimereply < lasttimereply)
		thistimereply = lasttimereply;

	return (lasttimereply = thistimereply);
}

/*
 * time in ms
 */
unsigned long system_getTime_realTime_ms()
{
	struct timeval tv;
	unsigned long thistimereply;

	gettimeofday(&tv, NULL);

	thistimereply = (tv.tv_sec * TICRATE * 1000 + (tv.tv_usec * TICRATE) / 1000);

	/* Fix for time problem */
	if (!basetime)
	{
		basetime = thistimereply;
		thistimereply = 0;
	}
	else
	{
		thistimereply -= basetime;
	}
	if (thistimereply < lasttimereply)
		thistimereply = lasttimereply;

	return (lasttimereply = thistimereply);
}



void randomize()
{
	srandom(1024);
}

int xrand(int hi)
{
	if(hi == 0)
		game_halt("Error: xrand: hi == 0.");
	return random() % hi;
}

