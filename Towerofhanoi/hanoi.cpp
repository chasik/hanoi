#include <windows.h>
#include <math.h>
//#include <conio.h>
#include <iostream> // strlen
#include "glut.h"

// мои заголовочные файлы


int **Rods;
int RingsCount = 0;
float animateRingToYY = 0, animateRingToXX = 0, topRod = 13;
float widthRing = 200, heightRing = 30;
int animatedRodNum = 0, animatedRingNum = 0;
float WinWidth = 1000, WinHeight = 500;

int rodUpNumber = -1, rodDownNumber = -1;

struct structMenuItem
{
	char name[40];
	char content[40];
	float posX, posY;
	float width, height;
};

structMenuItem menuArr[4] = { { "mode", "Режим" }, { "start", "Начать" }, { "load", "Загрузить" }, { "save", "Сохранить" } };

// --------------------------------------------------------------------------------------------------------------
// Функция обратного вызова для рисования сцены
// --------------------------------------------------------------------------------------------------------------
void RenderScene();
void Keyboard(unsigned char _key, int _x, int _y);
void AnimateRing(int _rodnum);
int GetCountRingAtRod(int _rodnum);
int GetValAnimatedRing(int _rodnum);
int GetIdAnimatedRing(int _rodnum);
void SwapRingsAtRods(int _rodstart, int _rodfinish);

int** InitData(int _countdisk);

void main(){
	Rods = InitData(RingsCount = 7);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WinWidth, WinHeight);
	glutInitWindowPosition(100, 200);
	glutCreateWindow("HaNoI");
	glutDisplayFunc(RenderScene);
	glutKeyboardFunc(Keyboard);
	glClearColor(0.85, 0.85, 0.85, 1); // цвет фона делаем серым (R=G=B)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, WinWidth, WinHeight, 0.0, -1.0, 1.0);
	char fileopt[] = "options.ini";
	glutMainLoop();
	return;
}

void RenderScene()
{
	float widthButton = 180, heightButton = 50;
	float widthRod = 12, heightRod = 380;
	float maxWidthRing = 220, minWidthRing = 120;
	float stepWidthRing = (maxWidthRing - minWidthRing) / RingsCount;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (int rodNumber = 0; rodNumber < 3; rodNumber++)
	{
		int i = rodNumber + 1;
		glBegin(GL_QUADS);
		glColor3f(0.6, 0.6, 0.6);
		glVertex2f(i * 250 - 100, 69);
		glVertex2f(i * 250 - 100, 69 + heightRod);
		glColor3f(0.3, 0.3, 0.3);
		glVertex2f(i * 250 - 100 + widthRod, 69 + heightRod);
		glVertex2f(i * 250 - 100 + widthRod, 69);
		glColor3f(0.6, 0.6, 0.6);
		glVertex2f(i * 250 - 217, 69 + heightRod);
		glVertex2f(i * 250 - 217, 73 + heightRod + widthRod);
		glColor3f(0.3, 0.3, 0.3);
		glVertex2f(i * 250 + 27, 73 + heightRod + widthRod);
		glVertex2f(i * 250 + 27, 69 + heightRod);
		glEnd();
	}
	int a[3][13];
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 13; j++)
			a[i][j] = Rods[i][j];
	}
	for (int rodNumber = 0; rodNumber < 3; rodNumber++)
	{
		int countringatrod = GetCountRingAtRod(rodNumber);
		for (int ringNumber = 0; ringNumber < countringatrod; ringNumber++)
		{
			int ringNum = Rods[rodNumber][ringNumber]; // ширина кольца (от 1 до N, где N - количество дисков)
			if (ringNum != 0)
			{
				int centerRod = rodNumber * 250 + 155;
				int animateYY = 0, animateXX = 0;
				float stepw = (RingsCount - ringNum - 1) * stepWidthRing;
				if (animatedRingNum == ringNum && animatedRodNum == rodNumber && rodUpNumber < 0)
					animateYY = animateRingToYY;
				if (animatedRingNum == ringNum && rodUpNumber == rodNumber)
				{
					animateXX = animateRingToXX;
					animateYY = animateRingToYY;
				}
				glBegin(GL_QUADS);
					glColor3f(0.3,0.3,0.4);
					glVertex2f(centerRod - widthRing / 2 + stepw + animateXX, 451 - ringNumber * heightRing - animateYY);
					glVertex2f(centerRod - widthRing / 2 + stepw + animateXX, 451 - (ringNumber + 1) * heightRing - animateYY);
					glVertex2f(centerRod + widthRing / 2 - stepw + animateXX, 451 - (ringNumber + 1) * heightRing - animateYY);
					glVertex2f(centerRod + widthRing / 2 - stepw + animateXX, 451 - (ringNumber)* heightRing - animateYY);

					glColor3f(0.04 * (RingsCount * ringNum) + 0.1, 0.03 * (RingsCount - ringNum) + 0.1, 0.02 * (RingsCount - ringNum) + 0.1);
					glVertex2f(centerRod - widthRing / 2 + stepw + 1 + animateXX, 451 - (ringNumber)* heightRing - 1 - animateYY);
					glVertex2f(centerRod - widthRing / 2 + stepw + 1 + animateXX, 451 - (ringNumber + 1) * heightRing + 1 - animateYY);
					glColor3f(0.04 * (RingsCount - ringNum) + 0.01, 0.03 * (RingsCount - ringNum ) + 0.01, 0.02 * (RingsCount - ringNum ) + 0.01);
					glVertex2f(centerRod + widthRing / 2 - stepw - 1 + animateXX, 451 - (ringNumber + 1) * heightRing + 1 - animateYY);
					glVertex2f(centerRod + widthRing / 2 - stepw - 1 + animateXX, 451 - (ringNumber)* heightRing - 1 - animateYY);
				glEnd();
			}
		}
	}

	for (int i = 0; i < sizeof(menuArr) / sizeof(structMenuItem); i++)
	{
		glColor3f(0, 0.443, 0.74);
		glBegin(GL_QUADS);
			glVertex2f(WinWidth - 10 - widthButton, i * heightButton + 10);
			glVertex2f(WinWidth - 10 - widthButton, (i + 1) * heightButton);
			glVertex2f(WinWidth - 10, (i + 1) * heightButton);
			glVertex2f(WinWidth - 10, i * heightButton + 10);
		glEnd();
		glBegin(GL_QUADS);
		glColor3f(0, 0.67, 0.887);
			glVertex2f(WinWidth - 10 - widthButton + 2, i * heightButton + 10 + 2);
			glColor3f(0, 0.4, 0.67);
			glVertex2f(WinWidth - 10 - widthButton + 2, (i + 1) * heightButton - 2);
			glVertex2f(WinWidth - 10 - 2, (i + 1) * heightButton - 2);
			glColor3f(0, 0.67, 0.887);
			glVertex2f(WinWidth - 10 - 2, i * heightButton + 10 + 2);
		glEnd();
		glColor3f(0, 0, 0);
		glRasterPos2f(WinWidth - 12 - (widthButton / 2) - strlen(menuArr[i].content) / 2 * 10, i * heightButton + 40);

		for (int j = 0; j < strlen(menuArr[i].content); j++)
		{
			int asciicode = menuArr[i].content[j] & 0xFF;
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, asciicode);

			//HWND dd = FindWindow(NULL, "HaNoI");
			//HDC dc = GetDC(dd);
			//TextOutA(dc, 10, 10, "asdfa", 0);
		}
	}

	glutSwapBuffers();
}

int** InitData(int _n)
{
	int **_rods;
	_rods = (int**)malloc(3 * sizeof(int*));
	for (int i = 0; i < 3; i++)
	{
		_rods[i] = (int*)malloc(13 * sizeof(int));
		for (int j = 0; j < RingsCount; j++)
		{
			if (i == 0)
				_rods[i][j] = _n - j > 0 ? _n - j : 0;
			else
				_rods[i][j] = 0;
		}
	}
	return _rods;
}

void AnimateRing(int _rodnum)
{
	animatedRodNum = _rodnum;
	glutPostRedisplay();
	if (rodUpNumber < 0)
	{
		if ((GetIdAnimatedRing(_rodnum) + 1) * heightRing + animateRingToYY < topRod * heightRing)
		{
			glutTimerFunc(10, AnimateRing, _rodnum);
			animateRingToYY += 30;
		}
		else
			rodUpNumber = _rodnum;
	}
	else if (rodDownNumber < 0)
	{
		if ((rodUpNumber < animatedRodNum && rodUpNumber * 250 + 155 + animateRingToXX < animatedRodNum * 250 + 155)
			|| (rodUpNumber > animatedRodNum && rodUpNumber * 250 + 155 + animateRingToXX > animatedRodNum * 250 + 155))
		{
			glutTimerFunc(10, AnimateRing, _rodnum);
			if (abs((animatedRodNum * 250 + 155) - (rodUpNumber * 250 + 155 + animateRingToXX)) > 30)
				animateRingToXX += 30 * (rodUpNumber > animatedRodNum ? -1 : 1);
			else
				animateRingToXX += 5 * (rodUpNumber > animatedRodNum ? -1 : 1);
		}
		else
		{
			rodDownNumber = _rodnum;
			glutTimerFunc(10, AnimateRing, _rodnum);
		}
	}
	else
	{
		if ((GetIdAnimatedRing(_rodnum) * heightRing + animateRingToYY > (GetCountRingAtRod(rodDownNumber) + 1) * heightRing) // опускание на стержень, отличный от стартового
			|| (rodUpNumber == rodDownNumber && (GetIdAnimatedRing(_rodnum) * heightRing + animateRingToYY >= (GetCountRingAtRod(rodDownNumber)) * heightRing))) // возврат на тот же (стартовый) стержень
		{
			glutTimerFunc(10, AnimateRing, _rodnum);
			animateRingToYY -= 30;
		}
		else
		{
			SwapRingsAtRods(rodUpNumber, rodDownNumber);
			rodUpNumber = -1;
			rodDownNumber = -1;
			animatedRingNum = -1;
			animatedRodNum = -1;
		}
	}
}

void SwapRingsAtRods(int _rodstart, int _rodfinish)
{
	int tempvalring = Rods[_rodstart][GetIdAnimatedRing(_rodstart)];
	Rods[_rodstart][GetIdAnimatedRing(_rodstart)] = 0;
	for (int i = 0; i < RingsCount; i++)
	{
		if (Rods[_rodfinish][i] == 0)
		{
			Rods[_rodfinish][i] = tempvalring;
			break;
		}
	}
}

int GetCountRingAtRod(int _rodnum)
{
	int _ringcount = 0;
	for (int i = 0; i < RingsCount; i++)
	{
		if (Rods[_rodnum][i] != 0)
			_ringcount++;
	}
	return _ringcount;
}

int GetValAnimatedRing(int _rodnum)
{
	for (int i = GetCountRingAtRod(_rodnum) - 1; i >= 0; i--)
	{
		if (Rods[_rodnum][i] != 0)
			return Rods[_rodnum][i];
	}
	return 0;
}

int GetIdAnimatedRing(int _rodnum)
{
	for (int i = GetCountRingAtRod(_rodnum) - 1; i > 0; i--)
	{
		if (Rods[_rodnum][i] == GetValAnimatedRing(_rodnum))
			return i;
	}
	return 0;
}

void Keyboard(unsigned char _key, int _x, int _y)
{
	if ((_key == '1') || (_key == '2') || (_key == '3'))
	{
		int keynum = 0;
		switch (_key)
		{
		case '1':
			keynum = 0;
			break;
		case '2':
			keynum = 1;
			break;
		case '3':
			keynum = 2;
			break;
		}
		if (rodUpNumber == -1) // если есть диск и не один диск еще не поднят
		{
			int a[3][13];
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 13; j++)
					a[i][j] = Rods[i][j];
			}
			animatedRingNum = GetValAnimatedRing(keynum);
			animateRingToYY = 0;
			animateRingToXX = 0;
			glutTimerFunc(10, AnimateRing, keynum);
		}
		else if (rodUpNumber != -1) // если есть диск и верхний уже поднят
		{
			animateRingToXX = 0;
			glutTimerFunc(10, AnimateRing, keynum);
		}
	}
}




