
#include <_gr2D.h>
#include <video.h>

Tgr2D gr2D;

/********отрисовка пиксела в буфер без учета окна********/
void gr2D_setpixel(int x, int y)
{
/*
asm
 xor    eax,eax
 mov    al,gr2D.color.current
 cmp    ax,gr2D.color.transparent
 je     @quit                                                         //если цвет прозрачный
 mov    edi,[x]
 cmp    edi,
 jae    @quit
 mov    eax,[y]
 cmp    eax,gr2D_SCR_sy
 jae    @quit
 shl    eax,6       //y=y*64
 add    edi,eax     //x=x+y
 shl    eax,2       //y=y*4
 add    edi,eax     //edi=x+y                                         //edi=y*320+x
 add    edi,gr2D.BUFcurrent
 mov    al,gr2D.color.current
 mov    byte ptr [edi],al
@quit:
*/
}

/********рисование линии********/
//x0,y0 - координаты начала линии
//x1,y1 - координаты конца линии
void gr2D_line (int x0, int y0, int x1, int y1)
{
/*
x,y:longint;
sh,sum:single;
 {
 sum = 0;
 if(x1=x0)and(y1=y0) gr2D_setpixel(x0,y0)
 else {
  if(abs(x1-x0)>=abs(y1-y0)) {
   if(x1=x0) sh = 0
   else {
    if(x1<x0) {
     x = x1;x1 = x0;x0 = x;
     y = y1;y1 = y0;y0 = y;
     }
    sh = (y1-y0)/(x1-x0);
    }
   if(x0<gr2D.WIN.x0) {
    sum = (gr2D.WIN.x0-x0)*sh;
    x0 = gr2D.WIN.x0;
    }
   if(x1>gr2D.WIN.x1) x1 = gr2D.WIN.x1;
   for x = x0 to x1 do {
    gr2D_setpixel(x,y0+round(sum));
    sum = sum+sh;
    }
   }
  else {
   if(y1=y0) sh = 0
   else {
    if(y1<y0) {
     x = x1;x1 = x0;x0 = x;
     y = y1;y1 = y0;y0 = y;
     }
    sh = (x1-x0)/(y1-y0);
    }
   if(y0<gr2D.WIN.y0) {
    sum = (gr2D.WIN.y0-y0)*sh;
    y0 = gr2D.WIN.y0;
    }
   if(y1>gr2D.WIN.y1) y1 = gr2D.WIN.y1;
   for y = y0 to y1 do {
    gr2D_setpixel(x0+round(sum),y);
    sum = sum+sh;
    }
   }
  }
*/
}
/********рисование горизонтальной линии********/
void gr2D_line_h (int x0, int x1, int y)
{
/*
 asm
 mov    edx,[y]
 cmp    edx,gr2D_SCR_sy
 jae    @quit                                                         //проверим координату y
 mov    edi,[x0]
 mov    ecx,[x1]
 cmp    edi,ecx                                                       //сравним координаты x концов линии
 jle    @next0
 xchg   edi,ecx                                                       //обменяем если x1<x0
@next0:                                                               //edi=[x0] ecx=[x1]
 cmp    ecx,0                                                         //x1<0 => выход
 jl     @quit
 cmp    edi,                                               //gr3D_SCR_sy<x0 => выход
 jg     @quit
 cmp    edi,0
 jge    @next1
 mov    edi,0
@next1:
 cmp    ecx,
 jl     @next2
 mov    ecx,-1
@next2:
 sub    ecx,edi
 inc    ecx                                                           //количество пикселей в строке
 shl    edx,6       //y=y*64
 add    edi,edx     //x=x+y
 shl    edx,2       //y=y*4
 add    edi,edx     //edi=x+y                                         //edi=y*320+x
 add    edi,[gr2D.BUFcurrent]
 mov    bl,[gr2D.color.current]                                       //формируем двойное слово с цветом линии
 mov    bh,bl
 mov    eax,ebx
 shl    eax,16
 mov    ax,bx                                                         //теперь в EAX цвет заполнения
 push   ecx
 and    ecx,3                                                         //and ecx,00000011b
 rep    stosb
 pop    ecx
 shr    ecx,2                                                         //ecx=ecx>>2
 rep    stosd
@quit:
*/
}
/********рисование вертикальной линии********/
void gr2D_line_v (int x, int y0, int y1)
{
/*
 asm
 mov    eax,y1
 cmp    y0,eax
 jle    @gr2D_line_v_run
 xchg   y0,eax
 mov    y1,eax
@gr2D_line_v_run:
 mov    ecx,eax                                                       //eax - нижний конец линии
 mov    eax,y0
 sub    ecx,eax
 inc    ecx
@gr2D_line_v_again:
 push   ecx
 push   eax
 push   x
 push   eax
 call   gr2D_setpixel
 pop    eax
 inc    eax
 pop    ecx
 loop   @gr2D_line_v_again
*/
}
/*
 * рисование незаполненного прямоугольника
 * x0,y0 - координаты левой верхней вершины прямоугольника;
 * sx,sy - размер прямоугольника по OX и OY
 * */
void gr2D_rectangle_e(int x0, int y0, int sx, int sy)
{
/*
 if(sx=0)or(sy=0) exit;
 sx = sx-1;
 sy = sy-1;
 if(sx=0)or(sy=0) exit;
 gr2D_line_h(x0   ,x0+sx,y0   );
 gr2D_line_h(x0   ,x0+sx,y0+sy);
 gr2D_line_v(x0   ,y0   ,y0+sy);
 gr2D_line_v(x0+sx,y0   ,y0+sy);
*/
}
/********рисование прямоугольника заполненного цветом grXD.color.current********/
//x0,y0 - координаты левой верхней вершины прямоугольника
//sx,sy - размер прямоугольника по OX и OY
void gr2D_rectangle_f(int x0, int y0, int sx, int sy)
{
/*
var count:longint;

 if(sx=0)or(sy=0) exit;
 sx = sx-1;
 sy = sy-1;
 for count = 0 to sx do gr2D_line_v(x0+count,y0,y0+sy);
*/
}

/********вывод цветной линии с цветами определенными в bytemap********/
void gr2D_setline(int x, int y, int length, char * bytemap)
{
/* asm

 mov    ebx,[y]                                                       //если X,Y выходит за
 cmp    ebx,gr2D.WIN.y0                                               // правую или верхнюю
jl      @gr2D_setline_}                                             // или нижнюю
 cmp    ebx,gr2D.WIN.y1                                               // границу окна,
jg      @gr2D_setline_}                                             // тогда не рисуем
 mov    eax,[x]                                                       // линию
 cmp    eax,gr2D.WIN.x1                                               // т.е. выходим
jg      @gr2D_setline_}                                             // из процедуры
 mov    esi,bytemap                                                   //esi = указатель на изображение
 mov    ecx,length                                                    //ecx = длина линии
 mov    edx,eax                                                       //eax=x
 add    edx,ecx
 dec    edx                                                           //edx = правая граница
 cmp    eax,gr2D.WIN.x0
jge     @gr2D_setline_correct                                         //уходим на корректировку размера
 sub    eax,gr2D.WIN.x0                                               //корректируем начало
 sub    esi,eax                                                       // буфера чтения
 mov    eax,gr2D.WIN.x0
 mov    ecx,edx
 sub    ecx,eax
 inc    ecx
@gr2D_setline_correct:
 cmp    edx,gr2D.WIN.x0                                               //правая граница линии меньше
jl      @gr2D_setline_}                                             // левой границы окна? выходим.
 cmp    edx,gr2D.WIN.x1
jle     @gr2D_setline_{                                           //уходим на рисование линии
 mov    ecx,gr2D.WIN.x1
 sub    ecx,eax
 inc    ecx
@gr2D_setline_{:
 mov    edi,eax
 shl    ebx,6
 add    edi,ebx
 shl    ebx,2
 add    edi,ebx                                                       //edi=y*320+x
 add    edi,gr2D.BUFcurrent
@gr2D_setline_loop:
 movzx  eax,byte ptr[esi]
 cmp    ax,gr2D.color.transparent
je      @gr2D_setline_transp                                          //если цвет прозрачный
 mov    byte ptr [edi],al
@gr2D_setline_transp:
 inc    edi
 inc    esi
 loop   @gr2D_setline_loop
@gr2D_setline_}:
*/
}
/*
 * вывод байтового образа bytemap размерами full_x,full_y, в позицию
 * out_x,out_y на экран с позиции get_x,get_y, размером sx,sy
 */
void gr2D_setimage0(
	int out_x,
	int out_y,
	item_img_t * image
)
{
	GLfloat scalex = VIDEO_MODE_W / 320.0f;
	GLfloat scaley = VIDEO_MODE_H / 200.0f;

	GLfloat mdl_sx = image->IMG->sx * scalex;
	GLfloat mdl_sy = image->IMG->sy * scaley;

	GLfloat texture_sx = image->sx;
	GLfloat texture_sy = image->sy;

	GLfloat texture_x1 = image->IMG->sx/texture_sx;
	GLfloat texture_y1 = image->IMG->sy/texture_sy;

	glBindTexture(GL_TEXTURE_2D, image->textures);
	glLoadIdentity();
	glTranslatef(out_x * scalex, out_y * scaley, 0.0f);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(texture_x1, texture_y1); glVertex2f(mdl_sx, mdl_sy); // Верхний правый угол квадрата
	glTexCoord2f(texture_x1, 0.0f      ); glVertex2f(mdl_sx, 0.0f  ); // Нижний правый
	glTexCoord2f(0.0f      , 0.0f      ); glVertex2f(0.0f  , 0.0f  ); // Нижний левый
	glTexCoord2f(0.0f      , texture_y1); glVertex2f(0.0f  , mdl_sy); // Верхний левый
	glEnd();	// Закончили квадраты
}


/********вывод байтового образа bytemap размерами full_x,full_y, в позицию
        out_x,out_y на экран с позиции get_x,get_y, размером sx,sy       ********/
void gr2D_setimage1(
	int out_x,
	int out_y,
	item_img_t * image,
	int get_x,
	int get_y,
	int get_sx,
	int get_sy
	)
{
	GLfloat scalex = VIDEO_MODE_W / 320.0f;
	GLfloat scaley = VIDEO_MODE_H / 200.0f;

	GLfloat mdl_sx = get_sx * scalex;
	GLfloat mdl_sy = get_sy * scaley;
/*
	GLfloat mdl_sx = image->IMG->sx * scalex;
	GLfloat mdl_sy = image->IMG->sy * scaley;
*/
	GLfloat texture_sx = image->sx;
	GLfloat texture_sy = image->sy;



	GLfloat texture_x0 = get_x/texture_sx;
	GLfloat texture_y0 = get_y/texture_sy;
	GLfloat texture_x1 = texture_x0+(get_sx/texture_sx);
	GLfloat texture_y1 = texture_y0+(get_sy/texture_sy);
/*
	GLfloat texture_x1 = image->IMG->sx/texture_sx;
	GLfloat texture_y1 = image->IMG->sy/texture_sy;
*/

	glBindTexture(GL_TEXTURE_2D, image->textures);
	glLoadIdentity();
	glTranslatef(out_x * scalex, out_y * scaley, 0.0f);
	glBegin(GL_QUADS);		// Рисуем куб
	glColor3f(1.0f, 1.0f, 1.0f);

	glTexCoord2f(texture_x1, texture_y1); glVertex2f(mdl_sx, mdl_sy); // Верхний правый угол квадрата
	glTexCoord2f(texture_x1, texture_y0); glVertex2f(mdl_sx, 0.0f  ); // Нижний правый
	glTexCoord2f(texture_x0, texture_y0); glVertex2f(0.0f  , 0.0f  ); // Нижний левый
	glTexCoord2f(texture_x0, texture_y1); glVertex2f(0.0f  , mdl_sy); // Верхний левый

	glEnd();	// Закончили квадраты
}
