#include <video.h>
#include <_gr2D.h>
#include <fonts.h>

#include <game.h>

#include <math.h>

#include <fttypes.h>
#include <ftimage.h>
#include FT_FREETYPE_H
#include <freetype.h>
#include <ft2build.h>
/*
#include <freetype2/freetype.h>
#include <fontconfig/fcfreetype.h>
*/


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
#define CHAR_AMOUNT (128)
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

//#define CHAR_AMOUNT (2653)

#define CHAR_AMOUNT (128)

pairs_t * charpairs;

//character_t characters[CHAR_AMOUNT];



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
		if ( charmap->encoding == FT_ENCODING_UNICODE ) break;
	}
	if(!charmap)
	{
		game_halt("FT no charmap FT_ENCODING_UNICODE");
	}

	FT_Error err = FT_Set_Charmap(face, charmap);

	//int error = FT_Select_Charmap(face, encoding);
	//FT_ULong charcode = 0x1F028;//'П';
	//FT_UInt char_index = FT_Get_Char_Index(face, charcode);

	charpairs = pairs_init();
	int count = 0;
	FT_UInt agindex;
	FT_ULong charcode;
	charcode = FT_Get_First_Char( face, &agindex );
	while ( agindex != 0 && count < CHAR_AMOUNT)
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
			buf[iindex++] = (pindex == TRANSPARENT_COLOR_INDEX) ? 255 : OPAQUE;
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

		charcode = FT_Get_Next_Char( face, charcode, &agindex );


	}






}

void fonts_done()
{
FT_Done_Face(face);
FT_Done_FreeType(ft);
}
/*
 * рисование указанного символа
 */
int gr2Don_setchar(
	int x,
	int y,
	char ch
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


	character = (character_t *)pairs_get(charpairs, ch%128);
	if(!character) return 0;

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
	float grayColor = 0.35f;
	glColor3f(grayColor, grayColor, grayColor);
	glTexCoord2f(texture_x1, texture_y1); glVertex2f(w, h); // Верхний правый угол квадрата
	glTexCoord2f(texture_x1, 0.0f      ); glVertex2f(w, 0.0f  ); // Нижний правый
	glTexCoord2f(0.0f      , 0.0f      ); glVertex2f(0.0f  , 0.0f  ); // Нижний левый
	glTexCoord2f(0.0f      , texture_y1); glVertex2f(0.0f  , h); // Верхний левый
	glEnd();	// Закончили квадраты
	//return character->advance;
	return (int) ( (float) character->advance * cscalex);
}


/*
 * вывод текста на экран
 */
void gr2Don_settext(
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

	int i = 0;
	int adv;

	while(string[i])
	{
		adv = gr2Don_setchar(x, y, string[i]);
		x += adv;
		i++;
	}
}
