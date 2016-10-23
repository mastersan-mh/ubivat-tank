/*
 * video.h
 *
 *  Created on: 16 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_VIDEO_H_
#define SRC_VIDEO_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_pixels.h>

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>


#define VIDEO_MODE_W 320
#define VIDEO_MODE_H 200

/*
#define VIDEO_MODE_W 640
#define VIDEO_MODE_H 480
*/
int video_init();
void video_done();

void video_screen_draw_begin();
void video_screen_draw_end();
void video_delay();

void video_viewport_set(
	float x0,
	float x1,
	float y0,
	float y1
);

void video_draw();

#endif /* SRC_VIDEO_H_ */
