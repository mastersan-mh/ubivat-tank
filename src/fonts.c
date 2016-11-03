#include <video.h>
#include <_gr2D.h>
#include <fonts.h>

#include <game.h>

#include <math.h>

#include <utf8.h>

#include <fttypes.h>
#include <ftimage.h>
#include FT_FREETYPE_H
#include <freetype.h>
#include <ft2build.h>
/*
#include <fontconfig/fcfreetype.h>
*/

#include <fontconfig/fontconfig.h>

/* unicode support */
#include <ctype.h>
#include <wctype.h>

typedef struct
{
	int key;
	void * val;
}pair_t;
typedef struct
{
	pair_t ** pairs;
	size_t amount;
	size_t size;
} pairs_t;

static void _pairs_mem_extend(pairs_t * pairs)
{
size_t size = pairs->size * 2;
pair_t ** tmp = realloc(pairs->pairs, size * sizeof(*tmp));
pairs->size = size;
pairs->pairs = tmp;
}


pairs_t * pairs_init()
{
	pairs_t * pairs = calloc(1, sizeof(* pairs));
	pairs->size = 32;
	_pairs_mem_extend(pairs);
	return pairs;
}


int pair_bsearch(
	size_t * index,
	const void * key,
	const void * base,
	size_t nmemb,
	size_t size,
	int (*compar)(const void *, const void *)
	)
{
	if(nmemb == 0)
	{
		*index = 0;
		return 0;
	}
	size_t a = 0;
	size_t b = nmemb;
	size_t mid = 0;
	ssize_t mid_prev = -1;
	while(mid != mid_prev)
	{
		mid_prev = mid;
		mid = (b+a) / 2;
		int ret = compar(key, (base + mid * size) );
		if(ret == 0)
		{
			*index = mid;
			return 1;
		}
		if(ret < 0) b = mid + 1;
		else a = mid;
	}
	*index = mid + 1;
	return 0;
}

int compare(const void * key, const void * data)
{
	const pair_t * __key = key;
	pair_t * const * __data = data;
	return __key->key - (*__data)->key;
}

int pairs_insert(pairs_t * pairs, int key, void * val, size_t size)
{
	int i;
	if(pairs->amount + 1 > pairs->size) _pairs_mem_extend(pairs);

	pair_t * ent = calloc(1, sizeof(* ent));
	ent->key = key;
	ent->val = calloc(1, size);
	memcpy(ent->val, val, size);

	size_t index;
	int found = pair_bsearch(&index, ent, pairs->pairs, pairs->amount, sizeof(pairs_t *), compare);
	if(found)return -1;
	if(pairs->amount > 0)
	{
		for(i = pairs->amount - 1; i > index ; i--)
		{
			pairs->pairs[i + 1] = pairs->pairs[i];
		}
	}
	pairs->pairs[index] = ent;
	pairs->amount++;
	return 0;
}


void * pairs_get(pairs_t * pairs, int key)
{
	size_t index;
	pair_t ent;
	ent.key = key;
	int found = pair_bsearch(&index, &ent, pairs->pairs, pairs->amount, sizeof(pairs_t *), compare);
	if(!found)return NULL;
	return pairs->pairs[index]->val;
}


#define FONTPATH "/usr/share/fonts/gnu-free/"
#define PLANES (4)
#define TRANSPARENT_COLOR_INDEX (0)

FT_Library ft;
FT_Face face;


typedef struct {
	// ID handle of the glyph texture
	GLuint textureId;
	// Size of glyph
	int size_x;
	int size_y;
	// Offset from baseline to left/top of glyph
	int bearing_x;
	int bearing_y;
	// Offset to advance to next glyph
	GLuint advance;
}character_t;

pairs_t * charpairs;


void fonts_test()
{

	/*
	SDL2/SDL_stdinc.h:extern DECLSPEC size_t SDLCALL SDL_utf8strlcpy(SDL_OUT_Z_CAP(dst_bytes) char *dst, const char *src, size_t dst_bytes);
	SDL2/SDL_stdinc.h:#define SDL_iconv_utf8_locale(S)    SDL_iconv_string("", "UTF-8", S, SDL_strlen(S)+1)
	SDL2/SDL_stdinc.h:#define SDL_iconv_utf8_ucs2(S)      (Uint16 *)SDL_iconv_string("UCS-2-INTERNAL", "UTF-8", S, SDL_strlen(S)+1)
	SDL2/SDL_stdinc.h:#define SDL_iconv_utf8_ucs4(S)      (Uint32 *)SDL_iconv_string("UCS-4-INTERNAL", "UTF-8", S, SDL_strlen(S)+1)
	SDL2/SDL_opengl_glext.h:#define GL_UTF8_NV                        0x909A
	SDL2/SDL_system.h:extern DECLSPEC const char * SDLCALL SDL_WinRTGetFSPathUTF8(SDL_WinRT_Path pathType);
	SDL2/SDL_hints.h: *  The contents of this hint should be encoded as a UTF8 string.

	fontconfig/fontconfig.h:FcUtf8ToUcs4 (const FcChar8 *src_orig,
	fontconfig/fontconfig.h:FcUtf8Len (const FcChar8    *string,
	fontconfig/fontconfig.h:#define FC_UTF8_MAX_LEN 6
	fontconfig/fontconfig.h:FcUcs4ToUtf8 (FcChar32  ucs4,
	fontconfig/fontconfig.h:              FcChar8   dest[FC_UTF8_MAX_LEN]);
	 */

	//char eng[] = "ABCD";
	//char rus[] = "АБВГ";
	//FcChar8 eng8[] = "ABCD";
	//FcChar8 rusBig8[]   = "АБВГДЕЁЖЗИйКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";

	char *rusSmall8[] =
	{
			"а",
			"аб",
			"абв",
			"абвг",
			"абвгд",
			"абвгде",
			"абвгдеё",
			"абвгдеёж",
			"абвгдеёжз",
			"абвгдеёжзи",
			"абвгдеёжзий",
			"абвгдеёжзийк",
			"абвгдеёжзийкл",
			"абвгдеёжзийклм",
			"абвгдеёжзийклмн",
			"абвгдеёжзийклмно",
			"абвгдеёжзийклмноп",
			"абвгдеёжзийклмнопр",
			"абвгдеёжзийклмнопрс",
			"абвгдеёжзийклмнопрст",
			"абвгдеёжзийклмнопрсту",
			"абвгдеёжзийклмнопрстуф",
			"абвгдеёжзийклмнопрстуфх",
			"абвгдеёжзийклмнопрстуфхц",
			"абвгдеёжзийклмнопрстуфхцч",
			"абвгдеёжзийклмнопрстуфхцчш",
			"абвгдеёжзийклмнопрстуфхцчшщ",
			"абвгдеёжзийклмнопрстуфхцчшщъ",
			"абвгдеёжзийклмнопрстуфхцчшщъы",
			"абвгдеёжзийклмнопрстуфхцчшщъыь",
			"абвгдеёжзийклмнопрстуфхцчшщъыьэ",
			"абвгдеёжзийклмнопрстуфхцчшщъыьэю",
			"абвгдеёжзийклмнопрстуфхцчшщъыьэюя",
			NULL
	};

	int i = 0;
	char * str8;
	int wchar;
	while((str8 =  rusSmall8[i++]) != NULL)
	{
		ssize_t len;
		len = utf8len(str8, &wchar);
		printf("str8 = %s; len = %ld; wchar = %d\n", str8, len, wchar);
		len = utf8nlen(str8, 99, &wchar);
		printf("str8 = %s; len = %ld; wchar = %d\n", str8, len, wchar);
	}

	char rus8[] =
			"АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"
			"абвгдеёжзийклмнопрстуфхцчшщъыьэюя";

	int handle(int i, uint32_t ucs4, int lchar, int wchar, void * ud)
	{
		union
		{
			uint32_t utf8;
			char str[5];
		} val;
		FcUcs4ToUtf8 (ucs4, (FcChar8 *) val.str);


		printf("i = %d; ucs4 = %d; utf8 = %d; lchar = %d; wchar = %d\n", i, ucs4, val.utf8, lchar, wchar);
		return 0;
	}

	int ret = utf8stringloop(rus8, handle, NULL);
	if(ret) exit(ret);
/*
	FcChar8 *pMyChar8 = argv[1];
	     FcChar32 myChar32;

	     FcUtf8ToUcs4(pMyChar8, &myChar32, strlen(pMyChar8));
	*/
	exit(0);
}

void fonts_init()
{

	if(FT_Init_FreeType(&ft))
		game_halt("Could not init freetype library");

	//if(FT_New_Face(ft, FONTPATH"FreeSerif.ttf", 0, &face)) {
	if(FT_New_Face(ft, FONTPATH"FreeSansBold.ttf", 0, &face)) {
		game_halt("Could not open font");
	}

	FT_Set_Pixel_Sizes(face, 0, 48);
	FT_Encoding encoding = FT_ENCODING_UNICODE;

	FT_CharMap charmap = NULL;
	int i;
	for ( i = 0; i < face->num_charmaps; i++ )
	{
		charmap = face->charmaps[i];
		if ( charmap->encoding == encoding ) break;
	}
	if(!charmap)
	{
		game_halt("FT no charmap FT_ENCODING_UNICODE");
	}

	FT_Error err = FT_Set_Charmap(face, charmap);
	if(err)
		game_halt("FT_Set_Charmap failed");

	//int error = FT_Select_Charmap(face, encoding);
	//FT_ULong charcode = 0x1F028;//'П';
	//FT_UInt char_index = FT_Get_Char_Index(face, charcode);

	int width = 1;

	charpairs = pairs_init();
	int count = 0;
	FT_UInt agindex;
	FT_ULong charcode;
	charcode = FT_Get_First_Char( face, &agindex );
	while ( agindex != 0)
	{
		//... do something with (charcode,gindex) pair ...

		if(FT_Load_Char(face, charcode, FT_LOAD_RENDER))
			game_halt("ERROR::FREETYTPE: Failed to load Glyph");

		/***************/

		int sx = face->glyph->bitmap.width;
		int sy = face->glyph->bitmap.rows;
		int size = sx * sy;
		uint8_t * buf = malloc(size * PLANES);


		int i = 0;
		int iindex = 0;
		float intencity = 1;
		for(i = 0; i < size; i++)
		{
			int pindex = face->glyph->bitmap.buffer[i];
			int color = pindex * intencity;
			buf[iindex++] = color;
			buf[iindex++] = color;
			buf[iindex++] = color;
			buf[iindex++] = (pindex == TRANSPARENT_COLOR_INDEX) ? COLOR_ALPHA_TRANSPARENT : COLOR_ALPHA_OPAQUE;
		}

		GLuint texture;
		glGenTextures(1, &texture);


		glBindTexture(GL_TEXTURE_2D, texture);
		if(buf)
		{
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA,
				sx,
				sy,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				buf
			);
		}
		if(buf)free(buf);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		character_t character = {
				.textureId = texture,
				.size_x    = sx,
				.size_y    = sy,
				.bearing_x = face->glyph->bitmap_left,
				.bearing_y = face->glyph->bitmap_top,
				.advance = face->glyph->advance.x >> 6
		};
		pairs_insert(charpairs, charcode, &character, sizeof(character));

		/*******************/

		//character_t character;
		//pair_insert(charpairs, charcode, &character);

		count++;

		printf("fonts_init(): chars = %ld (0x%lx)\n", charcode, charcode);
		if(charcode > 0x100) width = 2;

		charcode = FT_Get_Next_Char( face, charcode, &agindex );


	}

	printf("fonts_init(): total chars = %d\n", count);
	printf("fonts_init(): maxwidth = %d\n", width);


	/*********************************/




#define CHAR_AMOUNT (0)

	FT_ULong char_code;
	for (char_code = 0; char_code < CHAR_AMOUNT; char_code++)
	{

		// Load character glyph
		if (FT_Load_Char(face, char_code, FT_LOAD_RENDER))
		{
			game_halt("ERROR::FREETYTPE: Failed to load Glyph");
			continue;
		}

		int sx = face->glyph->bitmap.width;
		int sy = face->glyph->bitmap.rows;
		int size = sx * sy;
#define PLANES (4)
		uint8_t * buf = malloc(size * PLANES);

#define TRANSPARENT_COLOR_INDEX (0)

		int i = 0;
		int iindex = 0;
		float intencity = 1;
		for(i = 0; i < size; i++)
		{
			int pindex = face->glyph->bitmap.buffer[i];
			int color = pindex * intencity;
			buf[iindex++] = color;
			buf[iindex++] = color;
			buf[iindex++] = color;
		}
		GLuint texture;
		glGenTextures(1, &texture);

		glBindTexture(GL_TEXTURE_2D, texture);

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA,
			sx,
			sy,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			buf
		);

		free(buf);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		character_t character = {
				.textureId = texture,
				.size_x    = sx,
				.size_y    = sy,
				.bearing_x = face->glyph->bitmap_left,
				.bearing_y = face->glyph->bitmap_top,
				.advance = face->glyph->advance.x >> 6
		};

		pairs_insert(charpairs, charcode, &character, sizeof(character));

	}





	/*********************************/




}

void fonts_done()
{
FT_Done_Face(face);
FT_Done_FreeType(ft);
}

static uint8_t font_color[3];

void font_color_set3i(
	uint8_t r,
	uint8_t g,
	uint8_t b
)
{
	font_color[0] = r;
	font_color[1] = g;
	font_color[2] = b;
}

void font_color_sets(
	color_t * color
)
{
	font_color[0] = color->r;
	font_color[1] = color->g;
	font_color[2] = color->b;
}


/*
 * рисование указанного символа
 */
static void __video_print_char(
	int x,
	int y,
	uint32_t ch,
	int * adv_x,
	int * adv_y
)
{
/*
	int www = characters['A'].size_x;
	int hhh = characters['A'].size_y;
*/
	character_t * character;
	character = (character_t *)pairs_get(charpairs, 'A');
	if(!character)
		game_abort("gr2Don_setchar(): no character 'A'");
	int www = character->size_x;
	int hhh = character->size_y;

	float cscalex = (8.0f / www);
	float cscaley = (8.0f / hhh);

	int __adv_x;

	character = (character_t *)pairs_get(charpairs, ch);
	if(!character)
		__adv_x = www * cscalex;
	else
		__adv_x = (int)( (float) character->advance * cscalex);

	if(adv_x)
		*adv_x = __adv_x;
	if(adv_y)
		*adv_y = (int)( (float) hhh * cscaley);

	if(!character) return;


	// Size of glyph
	//GLfloat mdl_sx = character->size_width;
	//GLfloat mdl_sy = character->size_rows;
	// Offset from baseline to left/top of glyph
	//GLint texture_x1 = character->bearing_bitmap_left;
	//GLint texture_y1 = character->bearing_bitmap_top;

	GLint texture_x1 = 1.0f;
	GLint texture_y1 = 1.0f;

	// Offset to advance to next glyph

	GLfloat xpos = (x + (character->bearing_x * cscalex)) * VIDEO_SCALEX;

	//GLfloat ypos = y - (character->size_y - character->bearing_y) * scale;

	//GLfloat ypos = y - character->bearing_y * scale; // GOOD!!!

	//GLfloat ypos = y - character->bearing_y * scale + hhh * scale;

	GLfloat ypos = (y - ( character->bearing_y - hhh) * cscalex) * VIDEO_SCALEY;

	//GLfloat ypos = y - character->bearing_y * scale;

	//GLfloat ypos = y - 16 - (character->size_y) * scale;

	GLfloat w = character->size_x * cscalex * VIDEO_SCALEX;
	GLfloat h = character->size_y * cscaley * VIDEO_SCALEY;


	glBindTexture(GL_TEXTURE_2D, character->textureId);
	glLoadIdentity();
	glTranslatef(xpos, ypos, 0.0f);
	glBegin(GL_QUADS);
	glColor3f(font_color[0], font_color[1], font_color[2]);
	glTexCoord2f(texture_x1, texture_y1); glVertex2f(w, h); // Верхний правый угол квадрата
	glTexCoord2f(texture_x1, 0.0f      ); glVertex2f(w, 0.0f  ); // Нижний правый
	glTexCoord2f(0.0f      , 0.0f      ); glVertex2f(0.0f  , 0.0f  ); // Нижний левый
	glTexCoord2f(0.0f      , texture_y1); glVertex2f(0.0f  , h); // Верхний левый
	glEnd();
	//return character->advance;
}


void video_print_char(
	int x,
	int y,
	char ch
)
{
	__video_print_char(x, y, ch, NULL, NULL);
}


typedef struct
{
	int x;
	int y;
}video_printf_data_t;

static int video_printf_char_handle(int i, uint32_t c, int lchar, int wchar, void * ud)
{
	video_printf_data_t * __ud = ud;
	int adv_x;
	__video_print_char(__ud->x, __ud->y, c, &adv_x, NULL);
	__ud->x += adv_x;
	return 0;
}

/*
 * вывод текста на экран
 */
void video_printf(
	int x,
	int y,
	enum text_orient_e orientation,
	const char * format,
	...
)
{

	static char string[MAX_MESSAGE_SIZE];
	va_list argptr;
	va_start(argptr, format);
#ifdef HAVE_VSNPRINTF
	vsnprintf(string, MAX_MESSAGE_SIZE, format, argptr);
#else
	vsprintf(string, format, argptr);
#endif
	va_end(argptr);

	video_printf_data_t ud = {
			.x = x,
			.y = y
	};

	utf8nstringloop(string, MAX_MESSAGE_SIZE - 1, video_printf_char_handle, &ud);

}

typedef struct
{
	int start_x;
	int start_y;
	int x;
	int y;
	int pixels_width;
}video_printf_wide_data_t;

static int video_printf_wide_char_handle(int i, uint32_t c, int lchar, int wchar, void * ud)
{
	video_printf_wide_data_t * __ud = ud;
	int adv_x;
	int adv_y;
	__video_print_char(__ud->x, __ud->y, c, &adv_x, &adv_y);
	__ud->x += adv_x;
	if(__ud->x > __ud->pixels_width)
	{
		__ud->x = __ud->start_x;
		__ud->y += adv_y;
	}
	return 0;
}

/*
 * вывод текста на экран
 */
void video_printf_wide(
	int x,
	int y,
	int pixels_width,
	const char * format,
	...
)
{

	static char string[MAX_MESSAGE_SIZE];
	va_list argptr;
	va_start(argptr, format);
#ifdef HAVE_VSNPRINTF
	vsnprintf(string, MAX_MESSAGE_SIZE, format, argptr);
#else
	vsprintf(string, format, argptr);
#endif
	va_end(argptr);

	video_printf_wide_data_t ud = {
			.start_x = x,
			.start_y = y,
			.x = x,
			.y = y,
			.pixels_width = pixels_width * VIDEO_SCALEX
	};

	utf8nstringloop(string, MAX_MESSAGE_SIZE - 1, video_printf_wide_char_handle, &ud);

}
