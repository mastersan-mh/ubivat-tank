
// delete it

#include "game.h"
#include "_gr2D.h"
#include "video.h"

/*
static int list[14] =
{
		GL_ZERO,
		GL_ONE, // 1
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_DST_COLOR,

		GL_ONE_MINUS_DST_COLOR,
		GL_SRC_ALPHA,//6
		GL_ONE_MINUS_SRC_ALPHA,//7
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA,

		GL_CONSTANT_COLOR,
		GL_ONE_MINUS_CONSTANT_COLOR,
		GL_CONSTANT_ALPHA,
		GL_ONE_MINUS_CONSTANT_ALPHA
};
*/

/*
 * вывод байтового образа bytemap размерами full_x,full_y, в позицию
 * out_x,out_y на экран с позиции get_x,get_y, размером sx,sy
 */
void gr2D_setimage00(
	int out_x,
	int out_y,
	const item_img_t * image,
	int desired_sx,
	int desired_sy
)
{
	GLfloat mdl_sx;
	GLfloat mdl_sy;

	if(desired_sy > 0 && desired_sx > 0 )
	{
		mdl_sx = (GLfloat)desired_sx;
		mdl_sy = (GLfloat)desired_sy;
	}
	else if(desired_sx > 0)
	{
		mdl_sx = (GLfloat)desired_sx;
		mdl_sy = (GLfloat)image->img_sy * (GLfloat)desired_sx / (GLfloat)image->img_sx;
	}
	else if(desired_sy > 0)
	{
		mdl_sx = (GLfloat)image->img_sx * (GLfloat)desired_sy / (GLfloat)image->img_sy;
		mdl_sy = (GLfloat)desired_sy;
	}
	else
	{
		mdl_sx = image->img_sx;
		mdl_sy = image->img_sy;
	}

	mdl_sx *= VIDEO_SCALEX;
	mdl_sy *= VIDEO_SCALEY;

	GLfloat texture_x1 = (GLfloat)image->img_sx / (GLfloat)image->texture_sx;
	GLfloat texture_y1 = (GLfloat)image->img_sy / (GLfloat)image->texture_sy;

/*
	int sfactor = list[game_video_sfactor];
	int dfactor = list[game_video_dfactor];
	glBlendFunc(sfactor, dfactor);
*/

	glBlendFunc(image->sfactor, image->dfactor);

	glBindTexture(GL_TEXTURE_2D, image->texture);
	glLoadIdentity();
	glTranslatef(out_x * VIDEO_SCALEX, out_y * VIDEO_SCALEY, 0.0f);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
//	glColor3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(texture_x1, texture_y1); glVertex2f(mdl_sx, mdl_sy); // Верхний правый угол квадрата
	glTexCoord2f(texture_x1, 0.0f      ); glVertex2f(mdl_sx, 0.0f  ); // Нижний правый
	glTexCoord2f(0.0f      , 0.0f      ); glVertex2f(0.0f  , 0.0f  ); // Нижний левый
	glTexCoord2f(0.0f      , texture_y1); glVertex2f(0.0f  , mdl_sy); // Верхний левый
	glEnd();
}

void gr2D_setimage_indexed(
	int x,
	int y,
	unsigned int sx,
	unsigned int sy,
	void * indexed_image
)
{
	/* DUMMY */
}

void gr2D_color_index(int index){}
void gr2D_line_h(int x, int y, int width){}
void gr2D_line_v(int x, int y, int height){}
void gr2D_rectangle_e(int x, int y, int sx, int sy){}
void gr2D_rectangle_f(int x, int y, int sx, int sy){}
void gr2D_setpixel(int x, int y){}

