#include <windows.h>
#include <stdio.h>
#include <GL.h>
#include "glprints.h"

GLuint  base;

GLvoid BuildFont(GLvoid)  // ���������� ������ ���������� ������
{
	HDC wgldc = wglGetCurrentDC();		// ��������� ��������� ���������� (windows API). ����� ��� ��������� ������.
	HFONT  font;					// ������������� ������
	base = glGenLists(224);			// ������� ����� ��� 224 ��������
	font = CreateFont(-22,			// ������ ������
		0,							// ������ ������
		0,							// ���� ���������
		0,							// ���� �������
		FW_BOLD,					// ������ ������
		FALSE,						// ������
		FALSE,						// �������������
		FALSE,						// ��������������
		RUSSIAN_CHARSET,			// ������������� ������ ��������
		OUT_TT_PRECIS,				// �������� ������
		CLIP_DEFAULT_PRECIS,		// �������� ���������
		ANTIALIASED_QUALITY,		// �������� ������
		FF_DONTCARE | DEFAULT_PITCH,  // ��������� � ���
		"Courier New"				// ��� ������
		);
	SelectObject(wgldc, font);					// ������� �����, ��������� ����
	wglUseFontBitmaps(wgldc, 32, 224, base);	// ��������� 224 �������� ������� � �������
}

GLvoid glPrint(const char *fmt, ...)        // ������� �������
{
	char text[256];							// ����� ��� ����� ������
	va_list ap;								// ��������� �� ������ ����������
	if (fmt == NULL)						// ���� ��� ������
		return;								// ������ �� ������
	va_start(ap, fmt);						// ������ ������ ����������
	vsprintf_s(text, fmt, ap);				// � ��������������� �������� � �������� ����
	va_end(ap);								// ��������� ���������� � ������
	glPushAttrib(GL_LIST_BIT);				// ����������� ���� ������ �����������
	glListBase(base - 32);					// ������ ���� ������� �
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text); // ����� �������� �����������
	glPopAttrib();							// ������� ����� ������ �����������
}

GLvoid KillFont(GLvoid)					// �������� ������
{
	glDeleteLists(base, 224);			// �������� ���� 224 ������� ����������� ( ����� )
}

