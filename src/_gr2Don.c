#include <_gr2D.h>
#include <_gr2Don.h>

#include <math.h>

const char fonttable[256][8] = {
{  0,  0,  0,  0,  0,  0,  0,  0},{126,129,165,129,189,153,129,126},
{126,255,219,255,195,231,255,126},{108,254,254,254,124, 56, 16,  0},
{ 16, 56,124,254,124, 56, 16,  0},{ 56,124, 56,254,254,214, 16, 56},
{ 16, 56,124,254,254,124, 16, 56},{  0,  0, 24, 60, 60, 24,  0,  0},
{255,255,231,195,195,231,255,255},{  0, 60,102, 66, 66,102, 60,  0},
{255,195,153,189,189,153,195,255},{ 15,  7, 15,125,204,204,204,120},
{ 60,102,102,102, 60, 24,126, 24},{ 63, 51, 63, 48, 48,112,240,224},
{127, 99,127, 99, 99,103,230,192},{ 24,219, 60,231,231, 60,219, 24},
{128,224,248,254,248,224,128,  0},{  2, 14, 62,254, 62, 14,  2,  0},
{ 24, 60,126, 24, 24,126, 60, 24},{102,102,102,102,102,  0,102,  0},
{127,219,219,123, 27, 27, 27,  0},{ 62, 97, 60,102,102, 60,134,124},
{  0,  0,  0,  0,126,126,126,  0},{ 24, 60,126, 24,126, 60, 24,255},
{ 24, 60,126, 24, 24, 24, 24,  0},{ 24, 24, 24, 24,126, 60, 24,  0},
{  0, 24, 12,254, 12, 24,  0,  0},{  0, 48, 96,254, 96, 48,  0,  0},
{  0,  0,192,192,192,254,  0,  0},{  0, 36,102,255,102, 36,  0,  0},
{  0, 24, 60,126,255,255,  0,  0},{  0,255,255,126, 60, 24,  0,  0},
{  0,  0,  0,  0,  0,  0,  0,  0},{ 24, 60, 60, 24, 24,  0, 24,  0},
{102,102, 36,  0,  0,  0,  0,  0},{108,108,254,108,254,108,108,  0},
{ 24, 62, 96, 60,  6,124, 24,  0},{  0,198,204, 24, 48,102,198,  0},
{ 56,108, 56,118,220,204,118,  0},{ 24, 24, 48,  0,  0,  0,  0,  0},
{ 12, 24, 48, 48, 48, 24, 12,  0},{ 48, 24, 12, 12, 12, 24, 48,  0},
{  0,102, 60,255, 60,102,  0,  0},{  0, 24, 24,126, 24, 24,  0,  0},
{  0,  0,  0,  0,  0, 24, 24, 48},{  0,  0,  0,126,  0,  0,  0,  0},
{  0,  0,  0,  0,  0, 24, 24,  0},{  6, 12, 24, 48, 96,192,128,  0},
{124,198,206,214,230,198,124,  0},{ 24, 56, 24, 24, 24, 24,126,  0},
{124,198,  6, 28, 48,102,254,  0},{124,198,  6, 60,  6,198,124,  0},
{ 28, 60,108,204,254, 12, 30,  0},{254,192,192,252,  6,198,124,  0},
{ 56, 96,192,252,198,198,124,  0},{254,198, 12, 24, 48, 48, 48,  0},
{124,198,198,124,198,198,124,  0},{124,198,198,126,  6, 12,120,  0},
{  0, 24, 24,  0,  0, 24, 24,  0},{  0, 24, 24,  0,  0, 24, 24, 48},
{  6, 12, 24, 48, 24, 12,  6,  0},{  0,  0,126,  0,  0,126,  0,  0},
{ 96, 48, 24, 12, 24, 48, 96,  0},{124,198, 12, 24, 24,  0, 24,  0},
{124,198,222,222,222,192,120,  0},{ 56,108,198,254,198,198,198,  0},
{252,102,102,124,102,102,252,  0},{ 60,102,192,192,192,102, 60,  0},
{248,108,102,102,102,108,248,  0},{254, 98,104,120,104, 98,254,  0},
{254, 98,104,120,104, 96,240,  0},{ 60,102,192,192,206,102, 58,  0},
{198,198,198,254,198,198,198,  0},{ 60, 24, 24, 24, 24, 24, 60,  0},
{ 30, 12, 12, 12,204,204,120,  0},{230,102,108,120,108,102,230,  0},
{240, 96, 96, 96, 98,102,254,  0},{198,238,254,254,214,198,198,  0},
{198,230,246,222,206,198,198,  0},{124,198,198,198,198,198,124,  0},
{252,102,102,124, 96, 96,240,  0},{124,198,198,198,198,206,124, 14},
{252,102,102,124,108,102,230,  0},{124,198, 96, 56, 12,198,124,  0},
{126,126, 90, 24, 24, 24, 60,  0},{198,198,198,198,198,198,124,  0},
{198,198,198,198,198,108, 56,  0},{198,198,198,214,214,254,108,  0},
{198,198,108, 56,108,198,198,  0},{102,102,102, 60, 24, 24, 60,  0},
{254,198,140, 24, 50,102,254,  0},{ 60, 48, 48, 48, 48, 48, 60,  0},
{192, 96, 48, 24, 12,  6,  2,  0},{ 60, 12, 12, 12, 12, 12, 60,  0},
{ 16, 56,108,198,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,255},
{ 48, 24, 12,  0,  0,  0,  0,  0},{  0,  0,120, 12,124,204,118,  0},
{224, 96,124,102,102,102,220,  0},{  0,  0,124,198,192,198,124,  0},
{ 28, 12,124,204,204,204,118,  0},{  0,  0,124,198,254,192,124,  0},
{ 60,102, 96,248, 96, 96,240,  0},{  0,  0,118,204,204,124, 12,248},
{224, 96,108,118,102,102,230,  0},{ 24,  0, 56, 24, 24, 24, 60,  0},
{  6,  0,  6,  6,  6,102,102, 60},{224, 96,102,108,120,108,230,  0},
{ 56, 24, 24, 24, 24, 24, 60,  0},{  0,  0,236,254,214,214,214,  0},
{  0,  0,220,102,102,102,102,  0},{  0,  0,124,198,198,198,124,  0},
{  0,  0,220,102,102,124, 96,240},{  0,  0,118,204,204,124, 12, 30},
{  0,  0,220,118, 96, 96,240,  0},{  0,  0,126,192,124,  6,252,  0},
{ 48, 48,252, 48, 48, 54, 28,  0},{  0,  0,204,204,204,204,118,  0},
{  0,  0,198,198,198,108, 56,  0},{  0,  0,198,214,214,254,108,  0},
{  0,  0,198,108, 56,108,198,  0},{  0,  0,198,198,198,126,  6,252},
{  0,  0,126, 76, 24, 50,126,  0},{ 14, 24, 24,112, 24, 24, 14,  0},
{ 24, 24, 24, 24, 24, 24, 24,  0},{112, 24, 24, 14, 24, 24,112,  0},
{118,220,  0,  0,  0,  0,  0,  0},{  0, 16, 56,108,198,198,254,  0},
{ 62,102,198,198,254,198,198,  0},{254,102, 96,124,102,102,252,  0},
{252,102,102,124,102,102,252,  0},{254,102, 96, 96, 96, 96,240,  0},
{ 30, 54,102,102,102,102,255,195},{254, 98,104,120,104, 98,254,  0},
{214,214,124, 56,124,214,214,  0},{124,198,  6, 60,  6,198,124,  0},
{198,198,206,222,246,230,198,  0},{ 56,198,206,222,246,230,198,  0},
{230,102,108,120,108,102,230,  0},{ 30, 54,102,102,102,102,198,  0},
{198,238,254,254,214,198,198,  0},{198,198,198,254,198,198,198,  0},
{124,198,198,198,198,198,124,  0},{254,198,198,198,198,198,198,  0},
{252,102,102,102,124, 96,240,  0},{124,198,192,192,192,198,124,  0},
{126, 90, 24, 24, 24, 24, 60,  0},{198,198,198,126,  6,198,124,  0},
{124,214,214,214,124, 16, 56,  0},{198,198,108, 56,108,198,198,  0},
{204,204,204,204,204,204,254,  6},{198,198,198,126,  6,  6,  6,  0},
{214,214,214,214,214,214,254,  0},{214,214,214,214,214,214,255,  3},
{240,240,176, 60, 54, 54, 60,  0},{198,198,198,246,222,222,246,  0},
{240, 96, 96,124,102,102,124,  0},{120,140,  6, 62,  6,140,120,  0},
{220,214,214,246,214,214,220,  0},{126,198,198,198,126,102,198,  0},
{  0,  0,120, 12,124,204,118,  0},{  6,124,192,124,198,198,124,  0},
{  0,  0,252,102,124,102,252,  0},{  0,  0,254,102, 96, 96,240,  0},
{  0,  0, 60,108,108,108,254,198},{  0,  0,124,198,254,192,124,  0},
{  0,  0,214,124, 56,124,214,  0},{  0,  0,124,198, 28,198,124,  0},
{  0,  0,198,206,222,246,230,  0},{  0, 56,198,206,222,246,230,  0},
{  0,  0,230,108,120,108,230,  0},{  0,  0, 62,102,102,102,230,  0},
{  0,  0,198,254,254,214,198,  0},{  0,  0,198,198,254,198,198,  0},
{  0,  0,124,198,198,198,124,  0},{  0,  0,254,198,198,198,198,  0},
{ 17, 68, 17, 68, 17, 68, 17, 68},{ 85,170, 85,170, 85,170, 85,170},
{221,119,221,119,221,119,221,119},{ 24, 24, 24, 24, 24, 24, 24, 24},
{ 24, 24, 24,248, 24, 24, 24, 24},{ 24,248, 24,248, 24, 24, 24, 24},
{ 54, 54, 54,246, 54, 54, 54, 54},{  0,  0,  0,254, 54, 54, 54, 54},
{  0,248, 24,248, 24, 24, 24, 24},{ 54,246,  6,246, 54, 54, 54, 54},
{ 54, 54, 54, 54, 54, 54, 54, 54},{  0,254,  6,246, 54, 54, 54, 54},
{ 54,246,  6,254,  0,  0,  0,  0},{ 54, 54, 54,254,  0,  0,  0,  0},
{ 24,248, 24,248,  0,  0,  0,  0},{  0,  0,  0,248, 24, 24, 24, 24},
{ 24, 24, 24, 31,  0,  0,  0,  0},{ 24, 24, 24,255,  0,  0,  0,  0},
{  0,  0,  0,255, 24, 24, 24, 24},{ 24, 24, 24, 31, 24, 24, 24, 24},
{  0,  0,  0,255,  0,  0,  0,  0},{ 24, 24, 24,255, 24, 24, 24, 24},
{ 24, 31, 24, 31, 24, 24, 24, 24},{ 54, 54, 54, 55, 54, 54, 54, 54},
{ 54, 55, 48, 63,  0,  0,  0,  0},{  0, 63, 48, 55, 54, 54, 54, 54},
{ 54,247,  0,255,  0,  0,  0,  0},{  0,255,  0,247, 54, 54, 54, 54},
{ 54, 55, 48, 55, 54, 54, 54, 54},{  0,255,  0,255,  0,  0,  0,  0},
{ 54,247,  0,247, 54, 54, 54, 54},{ 24,255,  0,255,  0,  0,  0,  0},
{ 54, 54, 54,255,  0,  0,  0,  0},{  0,255,  0,255, 24, 24, 24, 24},
{  0,  0,  0,255, 54, 54, 54, 54},{ 54, 54, 54, 63,  0,  0,  0,  0},
{ 24, 31, 24, 31,  0,  0,  0,  0},{  0, 31, 24, 31, 24, 24, 24, 24},
{  0,  0,  0, 63, 54, 54, 54, 54},{ 54, 54, 54,255, 54, 54, 54, 54},
{ 24,255, 24,255, 24, 24, 24, 24},{ 24, 24, 24,248,  0,  0,  0,  0},
{  0,  0,  0, 31, 24, 24, 24, 24},{255,255,255,255,255,255,255,255},
{  0,  0,  0,255,255,255,255,255},{240,240,240,240,240,240,240,240},
{ 15, 15, 15, 15, 15, 15, 15, 15},{255,255,255,  0,  0,  0,  0,  0},
{  0,  0,252,102,102,124, 96,240},{  0,  0,124,198,192,198,124,  0},
{  0,  0,126, 90, 24, 24, 60,  0},{  0,  0,198,198,198,126,  6,124},
{  0,  0,124,214,214,124, 16, 56},{  0,  0,198,108, 56,108,198,  0},
{  0,  0,204,204,204,204,254,  6},{  0,  0,198,198,126,  6,  6,  0},
{  0,  0,214,214,214,214,254,  0},{  0,  0,214,214,214,214,255,  3},
{  0,  0,240,176, 60, 54, 60,  0},{  0,  0,198,198,246,222,246,  0},
{  0,  0,240, 96,124,102,252,  0},{  0,  0,124,198, 30,198,124,  0},
{  0,  0,220,214,246,214,220,  0},{  0,  0,126,198,126,102,198,  0},
{108,254, 98,120, 96, 98,254,  0},{108,  0,124,198,254,192,124,  0},
{ 60,102,192,248,192,102, 60,  0},{  0,  0, 62, 99,120, 99, 62,  0},
{ 72,120, 48, 48, 48, 48,120,  0},{204,  0, 48, 48, 48, 48,120,  0},
{ 56,198,198,126,  6,198,124,  0},{108, 56,198,198,198,126,  6,124},
{ 56,108,108, 56,  0,  0,  0,  0},{  0,  0,  0, 24, 24,  0,  0,  0},
{  0,  0,  0,  0, 24,  0,  0,  0},{ 14, 12, 12, 12,108, 60, 28,  0},
{143,205,239,252,220,204,204,  0},{  0,198,124,198,198,124,198,  0},
{  0,  0, 60, 60, 60, 60,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0}
};

/*
 * рисование указанного символа
 */
void gr2Don_setchar(int px, int py, char ch)
{
/*
x,y:longint;
t_byte:byte;

 asm
  lea ebx,dword ptr[fonttable]

  mov cl,7
  sub cl,x
  mov dl,byte ptr[ebx]
  shl dl,cl
  cmp dl,0
  jne @next
  push px+x
  push py+y
  call gr2D_setpixel

  @next:

  }}


 for y = 0 to 7 do
  for x = 0 to 7 do {

   if((fonttable[byte(ch),y]and(1 shl(7-x))) != 0) gr2D_setpixel(px+x,py+y);


   }
*/
}
/*
 * вывод текста на экран
 */
void gr2Don_settext(
	int px,
	int py,
	enum text_orient_e orientation,
	const char * format,
	...
	)
{
/*
	c:byte;
	if(orientation=0) for c = 1 to byte(s[0]) do gr2Don_setchar(px+(c-1)*8,py        ,s[c])
	else              for c = 1 to byte(s[0]) do gr2Don_setchar(px        ,py+(c-1)*8,s[c]);
*/
}
