#ifndef _showinfo
#define _showinfo
	enum colorInfo
	{
		red = 0,
		blue = 1,
		green = 2
	};

	struct structShowInfo
	{
		int timer = 0;
		char *message;
		int timerlimit = 5000;
		colorInfo color = green;
	};

	void ShowInfo(structShowInfo *_showinf);													// вывод информации
	void ShowInfoInit(char *_infostr, structShowInfo *_showinf, colorInfo _color = blue);		// инициализация и начало отображения информационного сообщения
#endif


