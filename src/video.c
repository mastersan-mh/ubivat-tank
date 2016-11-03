
//#include <SDL2/SDL_image.h>
//#include <SDL2/SDL_mixer.h>

#include <types.h>
#include <video.h>

#include <fonts.h>

#include <inttypes.h>


#include <stdlib.h>
#include <stdio.h>

#define __STRING(x)	#x


void checkOpenGLError(const char* stmt, const char* fname, int line)
{
	struct errs_s
	{
		int value;
		char * name;
	} errs[] =
	{
			{-1              , "GL unknown error"},
			{GL_NO_ERROR     , "GL_NO_ERROR"      },
			{GL_INVALID_ENUM , "GL_INVALID_ENUM"  },
			{GL_INVALID_VALUE, "GL_INVALID_VALUE" },
			{GL_INVALID_OPERATION, "GL_INVALID_OPERATION"},
			{GL_STACK_OVERFLOW   , "GL_STACK_OVERFLOW"   },
			{GL_STACK_UNDERFLOW  , "GL_STACK_UNDERFLOW"  },
			{GL_OUT_OF_MEMORY    , "GL_OUT_OF_MEMORY"    }
	};
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		char * error_str = errs[0].name;
		int i;
		for(i = 0; i< sizeof(errs)/sizeof(*errs); i++)
		{
			struct errs_s * err = &errs[i];
			if(err->value == error)
			{
				error_str = err->name;
				break;
			}
		}
		printf("OpenGL error %08x, %s, at %s:%i - for %s\n", error, error_str, fname, line, stmt);
		abort();
	}
}



#define VIDEO_USE_OPENGL
//#define VIDEO_OPENGL_MAJOR_VERSION 1
//#define VIDEO_OPENGL_MINOR_VERSION 2

#define VIDEO_OPENGL_MAJOR_VERSION 1
#define VIDEO_OPENGL_MINOR_VERSION 2

#ifdef __A0
#define VIDEO_USE_SDL2
#endif


SDL_Window * window = NULL;

#if defined(VIDEO_USE_OPENGL)
SDL_GLContext glcontext = NULL;
#elif defined(VIDEO_USE_SDL2)
SDL_Renderer * renderer = NULL;
#endif

#define __SDL_PIXELFORMAT_AMOUNT 38
struct sdl_pixelformat_ent_s
{
	uint32_t value;
	char * name;
}
sdl_pixelformat_list[__SDL_PIXELFORMAT_AMOUNT] =
{
		{SDL_PIXELFORMAT_UNKNOWN  , "SDL_PIXELFORMAT_UNKNOWN"},
		{SDL_PIXELFORMAT_INDEX1LSB, "SDL_PIXELFORMAT_INDEX1LSB"},
		{SDL_PIXELFORMAT_INDEX1MSB, "SDL_PIXELFORMAT_INDEX1MSB"},
		{SDL_PIXELFORMAT_INDEX4LSB, "SDL_PIXELFORMAT_INDEX4LSB"},
		{SDL_PIXELFORMAT_INDEX4MSB, "SDL_PIXELFORMAT_INDEX4MSB"},
		{SDL_PIXELFORMAT_INDEX8   , "SDL_PIXELFORMAT_INDEX8"},
		{SDL_PIXELFORMAT_RGB332   , "SDL_PIXELFORMAT_RGB332"},
		{SDL_PIXELFORMAT_RGB444   , "SDL_PIXELFORMAT_RGB444"},
		{SDL_PIXELFORMAT_RGB555   , "SDL_PIXELFORMAT_RGB555"},
		{SDL_PIXELFORMAT_BGR555   , "SDL_PIXELFORMAT_BGR555"},
		{SDL_PIXELFORMAT_ARGB4444 , "SDL_PIXELFORMAT_ARGB4444"},
		{SDL_PIXELFORMAT_RGBA4444 , "SDL_PIXELFORMAT_RGBA4444"},
		{SDL_PIXELFORMAT_ABGR4444 , "SDL_PIXELFORMAT_ABGR4444"},
		{SDL_PIXELFORMAT_BGRA4444 , "SDL_PIXELFORMAT_BGRA4444"},
		{SDL_PIXELFORMAT_ARGB1555 , "SDL_PIXELFORMAT_ARGB1555"},
		{SDL_PIXELFORMAT_RGBA5551 , "SDL_PIXELFORMAT_RGBA5551"},
		{SDL_PIXELFORMAT_ABGR1555 , "SDL_PIXELFORMAT_ABGR1555"},
		{SDL_PIXELFORMAT_BGRA5551 , "SDL_PIXELFORMAT_BGRA5551"},
		{SDL_PIXELFORMAT_RGB565   , "SDL_PIXELFORMAT_RGB565"},
		{SDL_PIXELFORMAT_BGR565   , "SDL_PIXELFORMAT_BGR565"},
		{SDL_PIXELFORMAT_RGB24    , "SDL_PIXELFORMAT_RGB24"},
		{SDL_PIXELFORMAT_BGR24    , "SDL_PIXELFORMAT_BGR24"},
		{SDL_PIXELFORMAT_RGB888   , "SDL_PIXELFORMAT_RGB888"},
		{SDL_PIXELFORMAT_RGBX8888 , "SDL_PIXELFORMAT_RGBX8888"},
		{SDL_PIXELFORMAT_BGR888   , "SDL_PIXELFORMAT_BGR888"},
		{SDL_PIXELFORMAT_BGRX8888 , "SDL_PIXELFORMAT_BGRX8888"},
		{SDL_PIXELFORMAT_ARGB8888 , "SDL_PIXELFORMAT_ARGB8888"},
		{SDL_PIXELFORMAT_RGBA8888 , "SDL_PIXELFORMAT_RGBA8888"},
		{SDL_PIXELFORMAT_ABGR8888 , "SDL_PIXELFORMAT_ABGR8888"},
		{SDL_PIXELFORMAT_BGRA8888 , "SDL_PIXELFORMAT_BGRA8888"},
		{SDL_PIXELFORMAT_ARGB2101010, "SDL_PIXELFORMAT_ARGB2101010"},
		{SDL_PIXELFORMAT_YV12, "SDL_PIXELFORMAT_YV12"},      /**< Planar mode: Y + V + U  (3 planes) */
		{SDL_PIXELFORMAT_IYUV, "SDL_PIXELFORMAT_IYUV"},      /**< Planar mode: Y + U + V  (3 planes) */
		{SDL_PIXELFORMAT_YUY2, "SDL_PIXELFORMAT_YUY2"},      /**< Packed mode: Y0+U0+Y1+V0 (1 plane) */
		{SDL_PIXELFORMAT_UYVY, "SDL_PIXELFORMAT_UYVY"},      /**< Packed mode: U0+Y0+V0+Y1 (1 plane) */
		{SDL_PIXELFORMAT_YVYU, "SDL_PIXELFORMAT_YVYU"},      /**< Packed mode: Y0+V0+Y1+U0 (1 plane) */
		{SDL_PIXELFORMAT_NV12, "SDL_PIXELFORMAT_NV12"},      /**< Planar mode: Y + U/V interleaved  (2 planes) */
		{SDL_PIXELFORMAT_NV21, "SDL_PIXELFORMAT_NV21"},      /**< Planar mode: Y + V/U interleaved  (2 planes) */
};

char * SDL_PF_to_name(uint32_t value)
{
	struct sdl_pixelformat_ent_s * sp;
	for(int i = 0; i < __SDL_PIXELFORMAT_AMOUNT; i++)
	{
		sp = &sdl_pixelformat_list[i];
		if(sp->value == value) return sp->name;
	}
	return sdl_pixelformat_list[0].name;
}

/*
 *
 */
int Width = VIDEO_MODE_W;
int Height = VIDEO_MODE_H;

static void scene_init()
{

#if defined(VIDEO_USE_OPENGL)
	glClearColor(0.0, 0.0, 0.0, 0.0);
#elif defined(VIDEO_USE_SDL2)
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
#endif


	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);      // Разрешить тест глубины
	glDepthFunc(GL_LEQUAL);
	//glDisable(GL_DEPTH_TEST);
	// смешивание
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_SRC_ALPHA);

	glShadeModel(GL_SMOOTH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(45.0f, (float) VIDEO_MODE_W / (float) VIDEO_MODE_H, 0.1f, 100.0f); // настраиваем трехмерную перспективу


	Width = VIDEO_MODE_W;
	Height = VIDEO_MODE_H;
	gluOrtho2D(0.0, Width, Height, 0.0);

	glMatrixMode(GL_MODELVIEW); // переходим в трехмерный режим


	GL_CHECK(glEnable(GL_SCISSOR_TEST));




/*
	// Инициализация OpenGL
	glClearDepth(1.0);           // Установка буфера глубины

	//glDepthFunc(GL_LESS);
	glDepthFunc(GL_LEQUAL);       // Тип теста глубины

	glEnable(GL_DEPTH_TEST); // включаем тест глубины
	glEnable(GL_TEXTURE_2D); // разрешить наложение текстур
	glShadeModel(GL_SMOOTH); // Разрешение сглаженного закрашивания
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();


	GLdouble view_w = VIDEO_MODE_W;
	GLdouble view_h = VIDEO_MODE_H;
	GLdouble aspect = (GLdouble) VIDEO_MODE_W / (GLdouble) VIDEO_MODE_H;
	view_w = view_h * aspect;

	glFrustum(-view_w/2, view_w/2, -view_h/2, view_h/2, screen.near, 100.0f);
*/
	/*
    GLdouble view_w = 1;
    GLdouble view_h = 1;
    GLdouble view_near = 1;
    GLdouble aspect = (GLdouble) width / (GLdouble) height;
    view_w = view_h * aspect;
    view_near = (view_w+view_h)/2+0.04;
    glFrustum(-view_w/2, view_w/2, -view_h/2, view_h/2, view_near, 100.0f);
	 */

	//gluPerspective(45.0f, (float) width / (float) height, 1.0f, 100.0f); // настраиваем трехмерную перспективу
/*
	glMatrixMode(GL_MODELVIEW); // переходим в трехмерный режим
	glLoadIdentity();

	//    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Улучшенные вычисления перспективы

	//glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);    // Установка Фонового Света
	//glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);    // Установка Диффузного Света
	//glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);   // Позиция света
	//glEnable(GL_LIGHT1); // Разрешение источника света номер один
	//glEnable(GL_LIGHTING);

	glColor4f(1.0f,1.0f,1.0f,0.5f);   // Полная яркость, 50% альфа

	glBlendFunc(GL_SRC_ALPHA,GL_ONE); // Функция смешивания для непрозрачности,
	// базирующаяся на значении альфы(НОВОЕ)


*/
	/* fog
    glEnable(GL_FOG);                       // Включает туман (GL_FOG)
    glFogi(GL_FOG_MODE, fogMode[fogfilter]);// Выбираем тип тумана
    glFogfv(GL_FOG_COLOR, fogColor);        // Устанавливаем цвет тумана
    glFogf(GL_FOG_DENSITY, 0.15f);          // Насколько густым будет туман
    glHint(GL_FOG_HINT, GL_DONT_CARE);      // Вспомогательная установка тумана
    glFogf(GL_FOG_START, 1.0f);             // Глубина, с которой начинается туман
    glFogf(GL_FOG_END, 5.0f);               // Глубина, где туман заканчивается.
	 */

}


/*
 *
 */
int video_init()
{

	SDL_version compiled;
	SDL_version linked;

	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);
	printf("We compiled against SDL version %d.%d.%d ...\n",
		compiled.major, compiled.minor, compiled.patch
	);
	printf("But we are linking against SDL version %d.%d.%d.\n",
		linked.major, linked.minor, linked.patch
	);

	int ret;
	int i = 0;
	SDL_RendererInfo info;

	int renderAmount = SDL_GetNumRenderDrivers();
	if(renderAmount < 1)
	{
		printf("SDL_GetNumRenderDrivers Error: %s\n", SDL_GetError() );
		return -1;
	}


	for(int irender = 0; irender < renderAmount; irender++)
	{
		ret = SDL_GetRenderDriverInfo(i, &info);
		if(ret)
		{
			printf("SDL_GetRenderDriverInfo Error: %s\n", SDL_GetError() );
			return -1;
		}
		printf("\nname of the renderer: %s\n",
			info.name);
		printf("  Supported ::SDL_RendererFlags: 0x%08x = %s | %s | %s | %s\n",
			info.flags,
			(info.flags & SDL_RENDERER_SOFTWARE      ? "SOFTWARE"      : "N/A"),
			(info.flags & SDL_RENDERER_ACCELERATED   ? "ACCELERATED"   : "N/A"),
			(info.flags & SDL_RENDERER_PRESENTVSYNC  ? "PRESENTVSYNC"  : "N/A"),
			(info.flags & SDL_RENDERER_TARGETTEXTURE ? "TARGETTEXTURE" : "N/A")
		);

		printf("  available texture formats: \n");
		for(int tf = 0; tf < info.num_texture_formats; tf++)
		{
			printf("    %d. %s\n", tf + 1, SDL_PF_to_name(info.texture_formats[tf]));
		}
		printf("  maximum texture width: %d\n",
			info.max_texture_width);
		printf("  maximum texture height: %d\n",
			info.max_texture_height);

		/*
		const char *name;           // The name of the renderer
		Uint32 flags;               // Supported ::SDL_RendererFlags
		Uint32 num_texture_formats; // The number of available texture formats
		Uint32 texture_formats[16]; // The available texture formats
		int max_texture_width;      // The maximum texture width
		int max_texture_height;     // The maximum texture height
		 */
		i++;
	}


	SDL_DisplayMode displayMode;
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL_Init Error: %s\n", SDL_GetError());
		return -1;
	}
	ret = SDL_GetDesktopDisplayMode(0, &displayMode);
	if(ret)
	{
		printf("SDL_GetDesktopDisplayMode Error: %s\n", SDL_GetError());
		return -1;
	}

	struct glattr_s
	{
		SDL_GLattr attr;
		int value;
	} glattrs[] =
	{
			/* core functions only */
			{SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE},
			{SDL_GL_DOUBLEBUFFER ,  1},
			{SDL_GL_DEPTH_SIZE   , 24},
			{SDL_GL_STENCIL_SIZE ,  8},
			{SDL_GL_CONTEXT_MAJOR_VERSION, VIDEO_OPENGL_MAJOR_VERSION},
			{SDL_GL_CONTEXT_MINOR_VERSION, VIDEO_OPENGL_MINOR_VERSION}

	};
	int size = sizeof(glattrs)/sizeof(*glattrs);
	for(i = 0; i < size; i++)
	{
		struct glattr_s *glattr = &glattrs[i];
		ret = SDL_GL_SetAttribute(glattr->attr, glattr->value);
		if(ret)
		{
			printf("SDL_GL_SetAttribute Error: %s\n", SDL_GetError());
			return -1;
		}
	}

	window = SDL_CreateWindow(
		"Ubivat Tank 2016",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		VIDEO_MODE_W,
		VIDEO_MODE_H,
#if defined(VIDEO_USE_OPENGL)
		SDL_WINDOW_OPENGL |
#endif
		SDL_WINDOW_SHOWN
		);
	if(!window)
	{
		printf("SDL_CreateWindow Error: %s\n", SDL_GetError() );
		return -1;
	}
#if defined(VIDEO_USE_OPENGL)
	glcontext = SDL_GL_CreateContext(window); // создаем контекст OpenGL
#elif defined(VIDEO_USE_SDL2)
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if(!renderer)
	{
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError() );
		return -1;
	}
#endif
	scene_init();
	fonts_init();
	return 0;
}

void video_done()
{
	fonts_done();
#if defined(VIDEO_USE_OPENGL)
	SDL_GL_DeleteContext(glcontext);
#elif defined(VIDEO_USE_SDL2)
	SDL_DestroyRenderer(renderer);
#endif
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void video_screen_draw_begin()
{
#if defined(VIDEO_USE_OPENGL)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#elif defined(VIDEO_USE_SDL2)
	SDL_RenderClear(renderer);
#endif
}
void video_screen_draw_end()
{
#if defined(VIDEO_USE_OPENGL)
	glFlush();
	SDL_GL_SwapWindow(window);
#elif defined(VIDEO_USE_SDL2)
	SDL_RenderPresent(renderer);
#endif
}

void video_delay()
{
	SDL_Delay(500);
}


void video_viewport_set(
	int x,
	int y,
	int sx,
	int sy
)
{

	//GL_CHECK(glScissor(x * VIDEO_SCALEX, y * VIDEO_SCALEY, sx * VIDEO_SCALEX, sy * VIDEO_SCALEY));

	int _y = y + (VIDEO_SCREEN_H - sy);
	GL_CHECK(glScissor(x * VIDEO_SCALEX, _y * VIDEO_SCALEY, sx * VIDEO_SCALEX, sy * VIDEO_SCALEY));

/*
	void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
	glEnable();
	glDisable();
	GL_SCISSOR_TEST

	 glClipPlane
	 (GLenum
	 plane
	,
	const GLdouble
	*
	equation
	)
*/

}

void video_draw()
{



	//drawCube(xrf, yrf, zrf);
/*
	glLoadIdentity();
	glBegin(GL_QUADS);
	// первая вершина будет находиться в начале координат
	glVertex2d(0, 0);
	glVertex2d(0, Height);
	glVertex2d(Width, Height);
	glVertex2d(Width, 0);
	glEnd();
*/



/*
		// An array of 3 vectors which represents 3 vertices
		static const GLfloat g_vertex_buffer_data[] = {
		   -1.0f, -1.0f, 0.0f,
		   1.0f, -1.0f, 0.0f,
		   0.0f,  1.0f, 0.0f,
		};
		// This will identify our vertex buffer
		GLuint vertexbuffer;
		// Generate 1 buffer, put the resulting identifier in vertexbuffer
		GL_CHECK(glGenBuffers(1, &vertexbuffer));
		// The following commands will talk about our 'vertexbuffer' buffer
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer));
		// Give our vertices to OpenGL.
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW));
*/
/*
		float points[] = {
		   0.0f,  0.5f,  0.0f,
		   0.5f, -0.5f,  0.0f,
		  -0.5f, -0.5f,  0.0f
		};
		GLuint vbo = 0;

		GL_CHECK(glGenBuffers(1, &vbo));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW));
*/


}

