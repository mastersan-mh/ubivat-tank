/*
 * video.h
 *
 *  Created on: 16 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_VIDEO_H_
#define SRC_VIDEO_H_

#include <types.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_pixels.h>

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

/*
#define VIDEO_MODE_W 320
#define VIDEO_MODE_H 200
*/

#define VIDEO_SCREEN_W 320
#define VIDEO_SCREEN_H 200

#define VIDEO_MODE_W 640
#define VIDEO_MODE_H 480



#define VIDEO_SCALEX ((float) ((float)VIDEO_MODE_W / (float)VIDEO_SCREEN_W))
#define VIDEO_SCALEY ((float) ((float)VIDEO_MODE_H / (float)VIDEO_SCREEN_H))
#define VIDEO_SCALE VIDEO_SCALEX

#define COLOR_ALPHA_OPAQUE 255
#define COLOR_ALPHA_TRANSPARENT 0

/*
#define COLOR_ALPHA_OPAQUE 0
#define COLOR_ALPHA_TRANSPARENT 255
*/

int video_init();
void video_done();

void video_screen_draw_begin();
void video_screen_draw_end();
void video_delay();

void video_viewport_set(
	float x,
	float y,
	float sx,
	float sy
);

void video_draw();

void checkOpenGLError(const char* stmt, const char* fname, int line);

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
		stmt; \
		checkOpenGLError(#stmt, __FILE__, __LINE__); \
} while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

#endif /* SRC_VIDEO_H_ */
