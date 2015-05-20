#include <windows.h>
#include <GL.h>
#include "showinfo.h"
#include "glprints.h"

extern structShowInfo stateShowInfo;

void ShowInfoInit(char *_infostr, structShowInfo *_showinf, colorInfo _color)
{
	(*_showinf).message = _infostr;
	(*_showinf).timer = 1;
	(*_showinf).timerlimit = 4;
	(*_showinf).color = _color;
}

void ShowInfo(structShowInfo *_showinf)
{
	switch ((*_showinf).color)
	{
	case red:
		glColor3f(0.9, 0.05, 0.05);
		break;
	case green:
		glColor3f(0.05, 0.9, 0.05);
		break;
	case blue:
		glColor3f(0.05, 0.05, 0.98);
		break;
	default:
		glColor3f(0.4, 0.4, 0.4);
		break;
	}
	glRasterPos2f(300, 26);
	glPrint((*_showinf).message);
	(*_showinf).timer++;
	if ((*_showinf).timer >= (*_showinf).timerlimit)// если время отображения информационной надписи больше чем установлено лимитом
		(*_showinf).timer = 0;						// то обнуляем таймер и тем самым убираем надпись с экрана
}
