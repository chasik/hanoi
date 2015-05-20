#include <windows.h>
#include <stdio.h>
#include <GL.h>
#include "glprints.h"

GLuint  base;

GLvoid BuildFont(GLvoid)  // Построение нашего растрового шрифта
{
	HDC wgldc = wglGetCurrentDC();		// получение контекста устройства (windows API). Нужен для установки шрифта.
	HFONT  font;					// Идентификатор шрифта
	base = glGenLists(224);			// Выделим место для 224 символов
	font = CreateFont(-22,			// Высота шрифта
		0,							// Ширина шрифта
		0,							// Угол отношения
		0,							// Угол наклона
		FW_BOLD,					// Ширина шрифта
		FALSE,						// Курсив
		FALSE,						// Подчеркивание
		FALSE,						// Перечеркивание
		RUSSIAN_CHARSET,			// Идентификатор набора символов
		OUT_TT_PRECIS,				// Точность вывода
		CLIP_DEFAULT_PRECIS,		// Точность отсечения
		ANTIALIASED_QUALITY,		// Качество вывода
		FF_DONTCARE | DEFAULT_PITCH,  // Семейство и шаг
		"Courier New"				// Имя шрифта
		);
	SelectObject(wgldc, font);					// Выбрать шрифт, созданный нами
	wglUseFontBitmaps(wgldc, 32, 224, base);	// Построить 224 символов начиная с пробела
}

GLvoid glPrint(const char *fmt, ...)        // функция «Печати»
{
	char text[256];							// Место для нашей строки
	va_list ap;								// Указатель на список аргументов
	if (fmt == NULL)						// Если нет текста
		return;								// Ничего не делать
	va_start(ap, fmt);						// Разбор строки переменных
	vsprintf_s(text, fmt, ap);				// И конвертирование символов в реальные коды
	va_end(ap);								// Результат помещается в строку
	glPushAttrib(GL_LIST_BIT);				// Протолкнуть биты списка отображения
	glListBase(base - 32);					// Задать базу символа в
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text); // Текст списками отображения
	glPopAttrib();							// Возврат битов списка отображения
}

GLvoid KillFont(GLvoid)					// Удаление шрифта
{
	glDeleteLists(base, 224);			// Удаление всех 224 списков отображения ( НОВОЕ )
}

