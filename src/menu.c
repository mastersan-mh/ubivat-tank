/*
 *        Ubivat Tank
 *        обработка меню
 * by Master San
 */
#include <defs.h>
#include <game.h>
#include <img.h>
#include <_gr2D.h>
#include <_gr2Don.h>
#include <x10_time.h>
#include <x10_str.h>
#include <x10_kbrd.h>

int menu_prelv();

/*
 * считывание статуса меню
 */
int menu_getmenustatus(int maxmenu, int * menu_cur)
{
	kbrd_readport();
	if(kbrd.port)
	{
		switch(kbrd.port)
		{
		case KP0_UPARROW_0   :
		case KP0_DOWNARROW_0 :
		case KP0_ENTER_0     :
		case KP0_ESCAPE_0    :
		case KP0_LEFTARROW_0 :
		case KP0_RIGHTARROW_0: game.allowpress = true;break;
		case KP0_UPARROW_1:
			if(game.allowpress)
			{
				game.allowpress = false;
				if(0<*menu_cur) (*menu_cur)--;
				return 0;
			};
			break;
		case KP0_DOWNARROW_1:
			if(game.allowpress) {
				game.allowpress = false;
				if(*menu_cur<maxmenu-1) (*menu_cur)++;
				return 0;
			};
			break;
		case KP0_ENTER_1:
			if(game.allowpress) {
				game.allowpress = false;
				return 1;
			};
			break;
		case KP0_ESCAPE_1:
			if(game.allowpress) {
				game.allowpress = false;
				return 2;
			};
			break;
		case KP0_LEFTARROW_1:
			if(game.allowpress) {
				game.allowpress = false;
				return 3;
			};
			break;
		case KP0_RIGHTARROW_1:
			if(game.allowpress) {
				game.allowpress = false;
				return 4;
			};
			break;
		}
	}
	return 0;
}
/*
 * главное меню
 */
int menu_main()
{
	int menu_cur = 0;
	bool quit = false;
	int state;
	int ret;
	while(!quit)
	{
		gr2D_setimage0(0    ,0            ,game.m_i_conback->IMG.sx,game.m_i_conback->IMG.sy,game.m_i_conback->IMG.pic);
		gr2D_setimage0(277  ,159          ,game.m_i_logo->IMG.sx   ,game.m_i_logo->IMG.sy   ,game.m_i_logo->IMG.pic   );
		gr2D.color.current = 1;
		gr2Don_settextZ(1     ,183          ,c_strTITLE);
		gr2Don_settextZ(1     ,191          ,c_strCORP);
		gr2D_setimage0(120,30+23*0        ,game.m_i_game   ->IMG.sx,game.m_i_game   ->IMG.sy,game.m_i_game   ->IMG.pic);
		gr2D_setimage0(120,30+23*1        ,game.m_i_case   ->IMG.sx,game.m_i_case   ->IMG.sy,game.m_i_case   ->IMG.pic);
		gr2D_setimage0(120,30+23*2        ,game.m_i_options->IMG.sx,game.m_i_options->IMG.sy,game.m_i_options->IMG.pic);
		gr2D_setimage0(120,30+23*3        ,game.m_i_about  ->IMG.sx,game.m_i_about  ->IMG.sy,game.m_i_about  ->IMG.pic);
		if(game.created)
			gr2D_setimage0(120,30+23*4        ,game.m_i_abort  ->IMG.sx,game.m_i_abort  ->IMG.sy,game.m_i_abort  ->IMG.pic);
		gr2D_setimage0(120,30+23*5        ,game.m_i_quit   ->IMG.sx,game.m_i_quit   ->IMG.sy,game.m_i_quit   ->IMG.pic);
		gr2D_setimage0( 97,30+23*menu_cur,game.m_i_cur_0   ->IMG.sx,game.m_i_cur_0  ->IMG.sy,game.m_i_cur_0  ->IMG.pic);
		state = menu_getmenustatus(6, &menu_cur);
		if(state == 1)
		{
			ret = (menu_cur+1)*0x10;
			quit = true;
		};
		if(state == 2) {
			ret = c_m_main;
			game.allowpress = false;
			if(game.created) game.ingame = true;
			quit = true;
		};
		gr2D_BUFcurrent2screen();
	}
	return ret;
}
/*
 * меню "ИГРА"
 */
int menu_game()
{
	int menu_cur = 0;
	bool quit = false;
	int state;
	int ret;
	while(!quit)
	{
		gr2D_setimage0(0  ,0             ,game.m_i_conback ->IMG.sx,game.m_i_conback ->IMG.sy,game.m_i_conback ->IMG.pic);
		gr2D_setimage0(120,30-23*1       ,game.m_i_game    ->IMG.sx,game.m_i_game    ->IMG.sy,game.m_i_game    ->IMG.pic);
		gr2D_setimage0(120,30+23*0       ,game.m_i_g_new_p1->IMG.sx,game.m_i_g_new_p1->IMG.sy,game.m_i_g_new_p1->IMG.pic);
		gr2D_setimage0(120,30+23*1       ,game.m_i_g_new_p2->IMG.sx,game.m_i_g_new_p2->IMG.sy,game.m_i_g_new_p2->IMG.pic);
		gr2D_setimage0(120,30+23*2       ,game.m_i_g_load  ->IMG.sx,game.m_i_g_load  ->IMG.sy,game.m_i_g_load  ->IMG.pic);
		gr2D_setimage0( 97,30+23*menu_cur,game.m_i_cur_0   ->IMG.sx,game.m_i_cur_0   ->IMG.sy,game.m_i_cur_0   ->IMG.pic);
		state = menu_getmenustatus(3, &menu_cur);
		if(state==1)
		{
			ret = menu_cur+1+c_m_game;
			quit = true;
		};
		if(state==2) {
			ret = 0;
			quit = true;
		};
		gr2D_BUFcurrent2screen();
	};
	return ret;
};


/*
 * меню "ЗАГРУЗКА"
 */
int menu_load()
{
	int menu_cur = 0;
	bool quit = false;
	int state;
	int ret;
	int c,cl;
	game_record_getsaves();
	while(!quit)
	{
		gr2D_setimage0(0  ,0              ,game.m_i_conback->IMG.sx,game.m_i_conback->IMG.sy,game.m_i_conback->IMG.pic);
		gr2D_setimage0(120,30+23*(-1)     ,game.m_i_g_load ->IMG.sx,game.m_i_g_load ->IMG.sy,game.m_i_g_load ->IMG.pic);
		for(cl = 0; cl < 8; cl++)
		{
			gr2D_setimage0(97+23          ,30+cl*15,game.m_i_lineL->IMG.sx,game.m_i_lineL->IMG.sy,game.m_i_lineL->IMG.pic);
			for(c = 0; c<=16;c++)
				gr2D_setimage0(97+23+4+8*c    ,30+cl*15,game.m_i_lineM->IMG.sx,game.m_i_lineM->IMG.sy,game.m_i_lineM->IMG.pic);
			gr2D_setimage0(97+23+4+8*(c+1),30+cl*15,game.m_i_lineR->IMG.sx,game.m_i_lineR->IMG.sy,game.m_i_lineR->IMG.pic);
			gr2D.color.current = 7;
			if(game.saveslist[cl].Hname[0] == 0xFF) gr2Don_settextZ(97+23+4,33+cl*15,"---===EMPTY===---");
			else
			{
				gr2Don_settextZ(97+23+4,33+cl*15,game.saveslist[cl].Hname);
				//отображение статуса сохраненной игры
				gr2D_setimage0(98+23+4+8*(c+1),29+cl*15,game.m_i_flagRUS->IMG.sx,game.m_i_flagRUS->IMG.sy,game.m_i_flagRUS->IMG.pic);
				if(game.saveslist[cl].flags & c_g_f_2PLAYERS)
				{
					gr2D_setimage0(110+23+4+8*(c+1),29+cl*15,game.m_i_flagRUS->IMG.sx,game.m_i_flagRUS->IMG.sy,game.m_i_flagRUS->IMG.pic);
				};
			};
		};
		gr2D_setimage0(97,30+15*menu_cur+2,game.m_i_cur_1   ->IMG.sx,game.m_i_cur_1  ->IMG.sy,game.m_i_cur_1  ->IMG.pic);
		state = menu_getmenustatus(8, &menu_cur);
		if(state==1)
		{
			if(game.created) ret = c_m_main;
			else
			{
				game.error = game_record_load(&game.saveslist[menu_cur]);
				if(!game.error) menu_prelv();
				else {
					if(2<=game.error) game_msg_error(game.error+10);
				};
				ret = c_m_main;
			};
			quit = true;
		};
		if(state == 2)
		{
			ret = c_m_game;
			quit = true;
		}
		gr2D_BUFcurrent2screen();
	}
	return ret;
}
/*
 * меню "СОХРАНЕНИЕ"
 */
int menu_save()
{
	int menu_cur = 0;
	int m_c      = 0;
	bool quit = false;
	int state;
	int c,cl;
	gamesave_t tmprec;
	bool f_input = false;
	char ch;
	long l;
	int ret;

	game_record_getsaves();
	while(!quit)
	{
		gr2D_setimage0(0  ,0              ,game.m_i_conback->IMG.sx,game.m_i_conback->IMG.sy,game.m_i_conback->IMG.pic);
		gr2D_setimage0(120,30+23*(-1)     ,game.m_i_g_save ->IMG.sx,game.m_i_g_save ->IMG.sy,game.m_i_g_save ->IMG.pic);
		if(!f_input) gr2D_setimage0(97,30+15*menu_cur+2,game.m_i_cur_1   ->IMG.sx,game.m_i_cur_1  ->IMG.sy,game.m_i_cur_1->IMG.pic);
		for(cl = 0; cl < 8; cl++)
		{
			gr2D_setimage0(97+23          ,30+cl*15,game.m_i_lineL->IMG.sx,game.m_i_lineL->IMG.sy,game.m_i_lineL->IMG.pic);
			for(c = 0; c<=16; c++)
				gr2D_setimage0(97+23+4+8*c    ,30+cl*15,game.m_i_lineM->IMG.sx,game.m_i_lineM->IMG.sy,game.m_i_lineM->IMG.pic);
			gr2D_setimage0(97+23+4+8*(c+1),30+cl*15,game.m_i_lineR->IMG.sx,game.m_i_lineR->IMG.sy,game.m_i_lineR->IMG.pic);
			gr2D.color.current = 7;
			if(game.saveslist[cl].Hname[0]==0xFF) gr2Don_settextZ(97+23+4,33+cl*15,"---===EMPTY===---");
			else
			{
				gr2Don_settextZ(97+23+4,33+cl*15,game.saveslist[cl].Hname);
				//отображение статуса сохраненной игры
				gr2D_setimage0( 98+23+4+8*(c+1),29+cl*15,game.m_i_flagRUS->IMG.sx,game.m_i_flagRUS->IMG.sy,game.m_i_flagRUS->IMG.pic);
				if(game.saveslist[cl].flags & c_g_f_2PLAYERS)
				{
					gr2D_setimage0(110+23+4+8*(c+1),29+cl*15,game.m_i_flagRUS->IMG.sx,game.m_i_flagRUS->IMG.sy,game.m_i_flagRUS->IMG.pic);
				}
			}
		}
		if(f_input)
		{
			kbrd.port = 0;
			kbrd_readport();
			if(kbrd.port>=0x80) game.allowpress = true;
			else
			{
				if( kbrd.port==KP0_ENTER_1 || kbrd.port == KP0_ESCAPE_1) game.allowpress = true;
				else
				{
					if(game.allowpress)
					{
						ch = (char)kbrd_char(0, 0, kbrd.port);
						l = strlen(game.saveslist[menu_cur].Hname);
						if(ch == 0xEE)
						{
							if(0<l) game.saveslist[menu_cur].Hname[l-1] = 0;
						}
						else
						{
							if(ch<0x80 && l<=16) strZ_addch(game.saveslist[menu_cur].Hname,ch);
						}
					}
					game.allowpress = false;
				}
			}
		}
		if(!f_input) state = menu_getmenustatus(8, &menu_cur);
		else         state = menu_getmenustatus(8, &m_c);
		if(state == 1)
		{
			if(f_input)
			{
				game_record_save(&game.saveslist[menu_cur]);
				f_input = false;
			}
			else
			{
				tmprec = game.saveslist[menu_cur];
				game.saveslist[menu_cur].flags = game.flags;
				if(game.saveslist[menu_cur].Hname[0]== 0xFF) game.saveslist[menu_cur].Hname[0] = 0x00;
				do{ kbrd_readport();}while(!( (kbrd.port==KP0_ENTER_0) ));
				f_input = true;
				};
			};
		if(state == 2)
		{
			if(f_input)
			{
				game.saveslist[menu_cur] = tmprec;
				f_input = false;
			}
			else
			{
				ret = 0;
				game.allowpress = false;
				if(game.created)
				{
					game.ingame = false;
					game_time_reset();
				}
				quit = true;
			}
		}
		gr2D_BUFcurrent2screen();
	}
	return ret;
}
/*
 * меню "ВЫБОР"
 */
int menu_case()
{
	int menu_cur= 0;
	bool quit = false;
	int state;
	int ret;
	while(!quit)
	{
		gr2D_setimage0(0  ,0             ,game.m_i_conback ->IMG.sx,game.m_i_conback ->IMG.sy,game.m_i_conback ->IMG.pic);
		gr2D_setimage0(120,30-23*1       ,game.m_i_case    ->IMG.sx,game.m_i_case    ->IMG.sy,game.m_i_case    ->IMG.pic);
		gr2D_setimage0(120,30+23*0       ,game.m_i_arrowL  ->IMG.sx,game.m_i_arrowL  ->IMG.sy,game.m_i_arrowL  ->IMG.pic);
		gr2D_setimage0(260,30+23*0       ,game.m_i_arrowR  ->IMG.sx,game.m_i_arrowR  ->IMG.sy,game.m_i_arrowR  ->IMG.pic);
		gr2D_setimage0(120,30+23*1       ,game.m_i_g_new_p1->IMG.sx,game.m_i_g_new_p1->IMG.sy,game.m_i_g_new_p1->IMG.pic);
		gr2D_setimage0(120,30+23*2       ,game.m_i_g_new_p2->IMG.sx,game.m_i_g_new_p2->IMG.sy,game.m_i_g_new_p2->IMG.pic);
		gr2D_setimage0( 97,30+23*menu_cur,game.m_i_cur_0   ->IMG.sx,game.m_i_cur_0   ->IMG.sy,game.m_i_cur_0   ->IMG.pic);
		gr2D.color.current = 25;
		gr2Don_settextZ(133,33+23*0,game.casemap->map);
		gr2Don_settextZ(133,41+23*0,game.casemap->name);
		state = menu_getmenustatus(3, &menu_cur);
		if(state == 1)
		{
			ret = menu_cur+1+c_m_case;
			quit = true;
		};
		if(state == 2)
		{
			ret = 0;
			quit = true;
		};
		if(menu_cur == 0) {
			if(state == 3)
			{//<
				if(game.casemap->prev) game.casemap = game.casemap->prev;
			};
			if(state == 4)
			{//>
				if(game.casemap->next) game.casemap = game.casemap->next;
			}
		}
		gr2D_BUFcurrent2screen();
	}
	return ret;
}
/*
 * меню "НАСТРОЙКИ"
 */
int menu_options()
{
	int menu_cur= 0;
	bool quit = false;
	int state;
	int ret;
	int cur_pl = 0;
	bool wait_a_key = false;
	char s[32];

	while(!quit)
	{
		gr2D_setimage0(0  ,0              ,game.m_i_conback->IMG.sx,game.m_i_conback->IMG.sy,game.m_i_conback->IMG.pic);
		gr2D_setimage0(120,30+23*(-1)     ,game.m_i_options->IMG.sx,game.m_i_options->IMG.sy,game.m_i_options->IMG.pic);
		if(!wait_a_key)
			gr2D_setimage0(58+cur_pl*131,30+12+12*menu_cur+1,game.m_i_cur_1   ->IMG.sx,game.m_i_cur_1  ->IMG.sy,game.m_i_cur_1  ->IMG.pic);
		gr2D.color.current = 25;
		gr2Don_settext( 58+131*0, 30+9*0, 0, "ИГРОК1");
		gr2Don_settext( 58+131*1, 30+9*0, 0, "ИГРОК2");
		gr2Don_settext( 9,32+12*1, 0, "Вперед");
		gr2Don_settext( 9,32+12*2, 0, "Назад");
		gr2Don_settext( 9,32+12*3, 0, "Влево");
		gr2Don_settext( 9,32+12*4, 0, "Вправо");
		gr2Don_settext( 9,32+12*5, 0, "Пульки");
		gr2Don_settext( 9,32+12*6, 0, "Ракета");
		gr2Don_settext( 9,32+12*7, 0, "Мина");
		gr2Don_settext(82+131*0,32+12*1,0,bytetostr(game.controlP0[ 0], s));
		gr2Don_settext(82+131*0,32+12*2,0,bytetostr(game.controlP0[ 2], s));
		gr2Don_settext(82+131*0,32+12*3,0,bytetostr(game.controlP0[ 4], s));
		gr2Don_settext(82+131*0,32+12*4,0,bytetostr(game.controlP0[ 6], s));
		gr2Don_settext(82+131*0,32+12*5,0,bytetostr(game.controlP0[ 8], s));
		gr2Don_settext(82+131*0,32+12*6,0,bytetostr(game.controlP0[10], s));
		gr2Don_settext(82+131*0,32+12*7,0,bytetostr(game.controlP0[12], s));
		gr2Don_settext(82+131*1,32+12*1,0,bytetostr(game.controlP1[ 0], s));
		gr2Don_settext(82+131*1,32+12*2,0,bytetostr(game.controlP1[ 2], s));
		gr2Don_settext(82+131*1,32+12*3,0,bytetostr(game.controlP1[ 4], s));
		gr2Don_settext(82+131*1,32+12*4,0,bytetostr(game.controlP1[ 6], s));
		gr2Don_settext(82+131*1,32+12*5,0,bytetostr(game.controlP1[ 8], s));
		gr2Don_settext(82+131*1,32+12*6,0,bytetostr(game.controlP1[10], s));
		gr2Don_settext(82+131*1,32+12*7,0,bytetostr(game.controlP1[12], s));
		if(wait_a_key && game.allowpress)
		{                //"забивание" клавиш
			do{ kbrd_readport();}while(!( (kbrd.port != KP0_ENTER_0) ));
			if(cur_pl==0)
			{                                             //настройка игрока 0
				game.controlP0[menu_cur*2+0] = kbrd.port;
				do{ kbrd_readport();}while(!( (kbrd.port != game.controlP0[menu_cur*2+0])));
				game.controlP0[menu_cur*2+1] = kbrd.port;
			}
			else
			{        //настройка игрока 1
				game.controlP1[menu_cur*2+0] = kbrd.port;
				do{ kbrd_readport();}while(!( (kbrd.port != game.controlP1[menu_cur*2+0]) ));
				game.controlP1[menu_cur*2+1] = kbrd.port;
			};
			wait_a_key = false;
		};
		state = menu_getmenustatus(7, &menu_cur);
		if(state==1) wait_a_key = true;       //нажали ENTER
		if(!wait_a_key)
		{
			if(state == 2)
			{
				ret = 0;
				quit = true;
			};
			if(state == 3) cur_pl = 0;
			if(state == 4) cur_pl = 1;
		};
		gr2D_BUFcurrent2screen();
	};
	game_cfg_save();
	return ret;
};
/*
 * меню "О ИГРЕ"
 */
int menu_about()
{
	int menu_cur= 0;
	bool quit = false;
	int state;
	int ret;

	while(!quit)
	{
		gr2D_setimage0(
				0,0,
				game.m_i_interlv->IMG.sx,
				game.m_i_interlv->IMG.sy,
				game.m_i_interlv->IMG.pic
		);
		gr2D.color.current = 13;
		gr2Don_settextZ( 8,10* 1,c_strTITLE);
		gr2Don_settextZ(56,10* 2,c_strCORP);
		gr2Don_settextZ( 8,10*15,c_about[12]);
		gr2Don_settextZ( 8,10*16,c_about[13]);
		gr2Don_settextZ( 8,10*17,c_about[14]);
		gr2Don_settextZ( 8,10*18,c_about[15]);

		gr2D.color.current = 7;
		gr2Don_settextZ( 8,10* 3,c_about[0]);
		gr2Don_settextZ( 8,10* 5,c_about[2]);
		gr2Don_settextZ( 8,10* 8,c_about[5]);
		gr2Don_settextZ( 8,10*10,c_about[7]);
		gr2Don_settextZ( 8,10*12,c_about[9]);
		gr2D.color.current = 15;
		gr2Don_settextZ( 8,10* 4,c_about[1]);
		gr2Don_settextZ( 8,10* 6,c_about[3]);
		gr2Don_settextZ( 8,10* 7,c_about[4]);
		gr2Don_settextZ( 8,10* 9,c_about[6]);
		gr2Don_settextZ( 8,10*11,c_about[8]);
		gr2Don_settextZ( 8,10*13,c_about[10]);

		state = menu_getmenustatus(1, &menu_cur);
		if(state==2) {
			ret = c_m_main;
			quit = true;
		};
		gr2D_BUFcurrent2screen();
	};
	return ret;
 };
/*
 * информация об уровне
 */
int menu_prelv()
{
	int menu_cur= 0;
	bool quit = false;
	int state;
	int ret;
	int count;

	game.allowpress = false;
	while(!quit)
	{
		gr2D_setimage0(0,0,game.m_i_interlv->IMG.sx,game.m_i_interlv->IMG.sy,game.m_i_interlv->IMG.pic);
		gr2D.color.current = 15;
		gr2Don_settextZ(160-06*4,8*5,"КАРТА:");
		gr2D.color.current = 15;
		gr2Don_settextZ(160-16*4,8*7,map.name);
		gr2Don_settextZ(160-07*4,8*10,"ЗАДАЧА:");
		gr2Don_settext(108,191,0,"НАЖМИ ПРОБЕЛ");
		count = 0;
		while(map.brief[count])
		{
			gr2Don_setchar(160-16*4+((count % 16)*8),8*12+((count / 16)*10),map.brief[count]);
			count++;
		};
		state = menu_getmenustatus(0, &menu_cur);
		if(state==2)
		{
			ret = c_m_main;
			quit = true;
		}
		else {
			kbrd_readport();
			if(kbrd.port==KP0_SPACE_1) {
				ret = c_m_main;
				quit = true;
			};
		};
		gr2D_BUFcurrent2screen();
	};
	return ret;
};
/*
 * заставка между уровнями
 */
int menu_interlv()
{
	int menu_cur= 0;
	bool quit = false;
	int state;
	int ret;
	game.allowpress = true;
	char s[32];
	while(!quit)
	{
		gr2D_setimage0(
				0,0,
				game.m_i_interlv->IMG.sx,
				game.m_i_interlv->IMG.sy,
				game.m_i_interlv->IMG.pic
		);
		gr2D.color.current = 15;
		gr2Don_settext(108,191,0,"НАЖМИ ПРОБЕЛ");
		if(!game.P1)
		{                                           //один игрок
			gr2D_setimage1(26,92,
					game.P0->Ibase->IMG.sx,
					game.P0->Ibase->IMG.sy,
					0,0,c_p_MDL_box,c_p_MDL_box,
					game.P0->Ibase->IMG.pic
			);
			gr2D.color.current = 15;
			gr2Don_settext(48+8*00,84      ,0,"ОЧКИ      ФРАГИ      ВСЕГО ФРАГОВ");
			gr2Don_settext(48+8*00,84+4+8*1,0,longinttostr(game.P0->charact.scores, s));
			gr2Don_settext(48+8*10,84+4+8*1,0,longinttostr(game.P0->charact.frags , s));
			gr2Don_settext(48+8*21,84+4+8*1,0,longinttostr(game.P0->charact.fragstotal, s));
		}
		else
		{                                                          //два игрока
			gr2D_setimage1(26,84+18*0-1,
					game.P0->Ibase->IMG.sx,
					game.P0->Ibase->IMG.sy,
					0,0,c_p_MDL_box,c_p_MDL_box,
					game.P0->Ibase->IMG.pic
			);
			gr2D_setimage1(26,84+18*1+1,
					game.P1->Ibase->IMG.sx,
					game.P1->Ibase->IMG.sy,
					0,0,c_p_MDL_box,c_p_MDL_box,
					game.P1->Ibase->IMG.pic
			);
			gr2D.color.current = 15;
			gr2Don_settext(48+8*00,76      -1,0,"ОЧКИ      ФРАГИ      ВСЕГО ФРАГОВ");
			gr2Don_settext(48+8*00,76+4+8*1-1,0,longinttostr(game.P0->charact.scores, s));
			gr2Don_settext(48+8*10,76+4+8*1-1,0,longinttostr(game.P0->charact.frags, s));
			gr2Don_settext(48+8*21,76+4+8*1-1,0,longinttostr(game.P0->charact.fragstotal, s));
			gr2Don_settext(48+8*00,76+4+8*3+1,0,longinttostr(game.P1->charact.scores, s));
			gr2Don_settext(48+8*10,76+4+8*3+1,0,longinttostr(game.P1->charact.frags, s));
			gr2Don_settext(48+8*21,76+4+8*3+1,0,longinttostr(game.P1->charact.fragstotal, s));
		};
		state = menu_getmenustatus(0, &menu_cur);
		if(state == 2)
		{
			ret = c_m_main;
			quit = true;
		}
		else
		{
			kbrd_readport();
			if(kbrd.port == KP0_SPACE_1)
			{
				ret = 0;
				game.allowpress = false;
				quit = true;
			}
		}
		gr2D_BUFcurrent2screen();
	}
	return ret;
}
