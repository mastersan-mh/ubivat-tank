/*
 * model_resources.c
 *
 *  Created on: 14 дек. 2016 г.
 *      Author: mastersan
 */

#include "model.h"
#include "img.h"
#include "model_resources.h"

#define VERTEXES_COMMON_NUM (4)
#define TRIANGLES_COMMON_NUM (2)
#define MODEL_FRAMES_COMMON_NUM (1)

/**
 * @description константа с координатами текстур, где кадры в текстуре располагаются вертикально
 * model_<prefix>
 * frame      - номер кадра = [0; frames_num)
 * frames_num - количество кадров
 */
#define MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(prefix, frame, frames_num) \
vec2_t model_texcoord_ ## prefix [VERTEXES_COMMON_NUM] =   \
{                                                          \
		{ 0.0f, ( (float)frame        ) / ( (float)frames_num ) }, \
		{ 0.0f, ( (float)frame + 1.0f ) / ( (float)frames_num ) }, \
		{ 1.0f, ( (float)frame + 1.0f ) / ( (float)frames_num ) }, \
		{ 1.0f, ( (float)frame        ) / ( (float)frames_num ) }  \
}

/**
 * common
 */
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame0 , 0, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame1 , 1, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame2 , 2, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame3 , 3, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame4 , 4, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame5 , 5, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame6 , 6, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame7 , 7, 8);

static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(16frame0 , 0, 16);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(16frame1 , 1, 16);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(16frame2 , 2, 16);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(16frame3 , 3, 16);


/* вершины, коордитаты и треугольники для модели по-умолчанию */
static vec2_t model_vertexes_common[VERTEXES_COMMON_NUM] =
{
		{-1.0f, -1.0f },
		{-1.0f,  1.0f },
		{ 1.0f,  1.0f },
		{ 1.0f, -1.0f }
};

static model_triangle_t model_trianges_common[TRIANGLES_COMMON_NUM] =
{
		{0, 1, 2 },
		{0, 2, 3 }
};

static vec2_t model_texcoord_common[VERTEXES_COMMON_NUM] =
{
		{ 0.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 0.0f }
};

static modelframe_t model_frames_common[MODEL_FRAMES_COMMON_NUM] =
{
		{ .texcoord = model_texcoord_common }
};


/**
 * exit
 */
static const model_t model_exit =
{
		.name = "exit",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_OBJ_EXIT,
		.fps = 0,
		.frames_num = MODEL_FRAMES_COMMON_NUM,
		.frames = model_frames_common
};

/**
 * bull_artillery
 */
static modelframe_t model_bull_artillery_frames[8] =
{
		{ .texcoord = model_texcoord_common }
};

static const model_t model_bull_artillery =
{
		.name = "bull_artillery",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_BULL_ARTILLERY,
		.fps = 0,
		.frames_num = 1,
		.frames = model_bull_artillery_frames
};

/**
 * bull_missile
 */

/*
В оригинальной текстуре ракеты 8 кадров, по 2 на каждое направление. Мы используем только 2 первых кадра.
*/
static modelframe_t model_bull_missile_frames[BULL_MISSILE_MODEL_FRAMES] =
{
		{ .texcoord = model_texcoord_8frame0 },
		{ .texcoord = model_texcoord_8frame1 }
};

static const model_t model_bull_missile =
{
		.name = "bull_missile",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_BULL_MISSILE,
		.fps = BULL_FPS,
		.frames_num = BULL_MISSILE_MODEL_FRAMES,
		.frames = model_bull_missile_frames
};

/**
 * bull_mine
 */

/* В оригинальной текстуре ракеты 8 кадров, по 2 на каждое направление. Мы используем только 2 первых кадра. */
static modelframe_t model_bull_mine_frames[BULL_MISSILE_MODEL_FRAMES] =
{
		{ .texcoord = model_texcoord_8frame0 },
		{ .texcoord = model_texcoord_8frame1 }
};

static const model_t model_bull_mine =
{
		.name = "bull_mine",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_BULL_MINE,
		.fps = BULL_FPS,
		.frames_num = BULL_MISSILE_MODEL_FRAMES,
		.frames = model_bull_mine_frames
};

/**
 * explode_small
 */

static modelframe_t model_explode_small_frames[EXPLODE_FRAMES_NUM] =
{
		{ .texcoord = model_texcoord_8frame0 },
		{ .texcoord = model_texcoord_8frame1 },
		{ .texcoord = model_texcoord_8frame2 },
		{ .texcoord = model_texcoord_8frame3 },
		{ .texcoord = model_texcoord_8frame4 },
		{ .texcoord = model_texcoord_8frame5 },
		{ .texcoord = model_texcoord_8frame6 },
		{ .texcoord = model_texcoord_8frame7 }
};

static const model_t model_explode_small =
{
		.name = "explode_small",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_EXPLODE_SMALL,
		.fps = EXPLODE_FPS,
		.frames_num = EXPLODE_FRAMES_NUM,
		.frames = model_explode_small_frames
};

/**
 * explode_big
 */

static modelframe_t model_explode_big_frames[8] =
{
		{ .texcoord = model_texcoord_8frame0 },
		{ .texcoord = model_texcoord_8frame1 },
		{ .texcoord = model_texcoord_8frame2 },
		{ .texcoord = model_texcoord_8frame3 },
		{ .texcoord = model_texcoord_8frame4 },
		{ .texcoord = model_texcoord_8frame5 },
		{ .texcoord = model_texcoord_8frame6 },
		{ .texcoord = model_texcoord_8frame7 }
};

static const model_t model_explode_big =
{
		.name = "explode_big",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_EXPLODE_BIG,
		.fps = EXPLODE_FPS,
		.frames_num = EXPLODE_FRAMES_NUM,
		.frames = model_explode_big_frames
};

/**
 * items
 */
static const model_t model_item_scores =
{
		.name = "item_scores",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_ITEM_STAR,
		.fps = 0,
		.frames_num = MODEL_FRAMES_COMMON_NUM,
		.frames = model_frames_common
};

static const model_t model_item_health =
{
		.name = "item_health",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_ITEM_HEALTH,
		.frames_num = MODEL_FRAMES_COMMON_NUM,
		.frames = model_frames_common
};

static const model_t model_item_armor =
{
		.name = "item_armor",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_ITEM_ARMOR,
		.frames_num = MODEL_FRAMES_COMMON_NUM,
		.frames = model_frames_common
};

static const model_t model_item_ammo_missile =
{
		.name = "item_ammo_missile",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_WEAPON_MISSILE,
		.frames_num = MODEL_FRAMES_COMMON_NUM,
		.frames = model_frames_common
};

static const model_t model_item_ammo_mine =
{
		.name = "item_ammo_mine",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_WEAPON_MINE,
		.frames_num = MODEL_FRAMES_COMMON_NUM,
		.frames = model_frames_common
};

/**
 * tank 1, 2, 3, 4, 5
 */
static modelframe_t model_tank_frames[TANK_FRAMES_NUM] =
{
		{ .texcoord = model_texcoord_16frame0 },
		{ .texcoord = model_texcoord_16frame1 },
		{ .texcoord = model_texcoord_16frame2 },
		{ .texcoord = model_texcoord_16frame3 }
};

static const model_t model_tank1 =
{
		.name = "tank1",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_TANK0, /* 16x256, 4 frames per direction */
		.fps = TANK_FPS,
		.frames_num = TANK_FRAMES_NUM,
		.frames = model_tank_frames
};

static const model_t model_tank2 =
{
		.name = "tank2",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_TANK1, /* 16x256, 4 frames per direction */
		.fps = TANK_FPS,
		.frames_num = TANK_FRAMES_NUM,
		.frames = model_tank_frames
};

static const model_t model_tank3 =
{
		.name = "tank3",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_TANK2, /* 16x256, 4 frames per direction */
		.fps = TANK_FPS,
		.frames_num = TANK_FRAMES_NUM,
		.frames = model_tank_frames
};

static const model_t model_tank4 =
{
		.name = "tank4",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_TANK3, /* 16x256, 4 frames per direction */
		.fps = TANK_FPS,
		.frames_num = TANK_FRAMES_NUM,
		.frames = model_tank_frames
};

static const model_t model_tank5 =
{
		.name = "tank5",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_TANK4, /* 16x256, 4 frames per direction */
		.fps = TANK_FPS,
		.frames_num = TANK_FRAMES_NUM,
		.frames = model_tank_frames
};

/**
 * flags 16x16
 */
static const model_t model_flag_player =
{
		.name = "flag_player",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_FLAG_RUS,
		.fps = 0,
		.frames_num = 1,
		.frames = model_frames_common
};

static const model_t model_flag_enemy =
{
		.name = "flag_enemy",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_FLAG_WHITE,
		.fps = 0,
		.frames_num = 1,
		.frames = model_frames_common
};

static const model_t model_flag_boss =
{
		.name = "flag_boss",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_FLAG_USA,
		.fps = 0,
		.frames_num = 1,
		.frames = model_frames_common
};

void model_resources_register(void)
{
	model_register(&model_exit);
	model_register(&model_bull_artillery);
	model_register(&model_bull_missile  );
	model_register(&model_bull_mine     );
	model_register(&model_explode_small);
	model_register(&model_explode_big  );
	model_register(&model_item_scores);
	model_register(&model_item_health);
	model_register(&model_item_armor );
	model_register(&model_item_ammo_missile);
	model_register(&model_item_ammo_mine   );
	model_register(&model_tank1);
	model_register(&model_tank2);
	model_register(&model_tank3);
	model_register(&model_tank4);
	model_register(&model_tank5);
	model_register(&model_flag_player);
	model_register(&model_flag_enemy);
	model_register(&model_flag_boss);

}
