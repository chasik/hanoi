#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <math.h>
//#include <conio.h>
#include <iostream> // strlen
#include "glut.h"

// мои заголовочные файлы

GLuint  base;
HDC wgldc;
int **Rods;

float WinWidth = 1000, WinHeight = 500;
float widthRing = 200, heightRing = 30;
float animateRingToYY = 0, animateRingToXX = 0, topRod = 13;
int animatedRodNum = 0, animatedRingNum = 0;

int rodUpNumber = -1, rodDownNumber = -1;

enum stateGame
{
	stop = 0,
	game = 1,
	pause = 2,
};

enum modeGame
{
	manual = 0,
	automatic = 1 
};

struct structState
{
	int RingsCount = 4;
	int nodeToSelectMenuItem = 0;
	float timercounter = 0;
	int movescount = 0;
	stateGame stategame = stop;
	modeGame modegame = manual;
};

structState stateProgram;

struct structMenuItem
{
	char name[20];
	char content[20];
	int statemenuitem;
	float posX, posY;
	float width, height;
};

structMenuItem menuArr[6] = { { "mode", "Ручками", 2 }
							, { "start", "Начать", 1 }
							, { "reset", "Сброс", 0 }
							, { "load", "Загрузить", 1 }
							, { "save", "Сохранить", 0 }
							, { "diskcount", "Дисков %u", 1 }
};

// --------------------------------------------------------------------------------------------------------------
// Функция обратного вызова для рисования сцены
// --------------------------------------------------------------------------------------------------------------
void RenderScene();

GLvoid BuildFont(GLvoid);
GLvoid KillFont(GLvoid);
GLvoid glPrint(const char *fmt, ...);

void Keyboard(unsigned char _key, int _x, int _y);
void SKeyboard(int key, int x, int y);
void AnimateRing(int _rodnum);
int GetCountRingAtRod(int _rodnum);
int GetValAnimatedRing(int _rodnum);
int GetIdAnimatedRing(int _rodnum);
void SwapRingsAtRods(int _rodstart, int _rodfinish);
bool MoveEnabled(int _rodnum);
void timerForGame(int _value);
void ShowTime();
void ShowMovesCount();
void ChangeRingsCount();

void StartNewGame();
void PauseGame();
void ResumeGame();
void ResetAllState();
void ChangeMode();

void SetSelectedMenuItem(int i);

int** InitData(int _countdisk);

void main(){
	Rods = InitData(stateProgram.RingsCount);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WinWidth, WinHeight);
	glutInitWindowPosition(100, 200);
	glutCreateWindow("HaNoI");
	wgldc = wglGetCurrentDC();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, WinWidth, WinHeight, 0.0, -1.0, 1.0);
	glutKeyboardFunc(Keyboard); // работа с клавиатурой (1,2,3)
	glutSpecialFunc(SKeyboard); // работа с клавиатурой (движение по меню с помощью стрелок)
	glClearColor(0.85, 0.85, 0.85, 1); // цвет фона делаем серым (R=G=B)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//char fileopt[] = "options.ini";
	glClearDepth(1.0f);         // Установка буфера глубины
	glEnable(GL_DEPTH_TEST);    // Разрешение теста глубины
	glDepthFunc(GL_LEQUAL);     // Тип теста глубины
	// Действительно хорошие вычисления перспективы
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	BuildFont();
	glutDisplayFunc(RenderScene);
	RenderScene();
	glutMainLoop();
	KillFont(); // удаляем шрифт
	return;
}

void RenderScene()
{
	float widthButton = 180, heightButton = 50;
	float widthRod = 12, heightRod = 380;
	float maxWidthRing = 220, minWidthRing = 120;
	float stepWidthRing = (maxWidthRing - minWidthRing) / stateProgram.RingsCount;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Очистка экран и буфера глубины

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
		bool rodMoveEnabled = MoveEnabled(rodNumber);

		int countringatrod = GetCountRingAtRod(rodNumber);
		for (int ringNumber = 0; ringNumber < countringatrod; ringNumber++)
		{
			int ringNum = Rods[rodNumber][ringNumber]; // ширина кольца (от 1 до N, где N - количество дисков)
			if (ringNum != 0)
			{
				int centerRod = rodNumber * 250 + 155;
				int animateYY = 0, animateXX = 0;
				float stepw = (stateProgram.RingsCount - ringNum - 1) * stepWidthRing;
				if (animatedRingNum == ringNum && animatedRodNum == rodNumber && rodUpNumber < 0)
					animateYY = animateRingToYY;
				if (animatedRingNum == ringNum && rodUpNumber == rodNumber)
				{
					animateXX = animateRingToXX;
					animateYY = animateRingToYY;
				}
				glBegin(GL_QUADS);
				glColor3f(0.3, 0.3, 0.4);
				glVertex2f(centerRod - widthRing / 2 + stepw + animateXX, 451 - ringNumber * heightRing - animateYY);
				glVertex2f(centerRod - widthRing / 2 + stepw + animateXX, 451 - (ringNumber + 1) * heightRing - animateYY);
				glVertex2f(centerRod + widthRing / 2 - stepw + animateXX, 451 - (ringNumber + 1) * heightRing - animateYY);
				glVertex2f(centerRod + widthRing / 2 - stepw + animateXX, 451 - (ringNumber)* heightRing - animateYY);

				rodMoveEnabled ? glColor3f(0.04 * (stateProgram.RingsCount * ringNum) + 0.1, 0.03 * (stateProgram.RingsCount - ringNum) + 0.1, 0.02 * (stateProgram.RingsCount - ringNum) + 0.1)
					: glColor3f(0.5, 0.5, 0.5);
				glVertex2f(centerRod - widthRing / 2 + stepw + 1 + animateXX, 451 - (ringNumber)* heightRing - 1 - animateYY);
				glVertex2f(centerRod - widthRing / 2 + stepw + 1 + animateXX, 451 - (ringNumber + 1) * heightRing + 1 - animateYY);
				rodMoveEnabled ? glColor3f(0.04 * (stateProgram.RingsCount - ringNum) + 0.01, 0.03 * (stateProgram.RingsCount - ringNum) + 0.01, 0.02 * (stateProgram.RingsCount - ringNum) + 0.01)
					: glColor3f(0.2, 0.2, 0.2);
				glVertex2f(centerRod + widthRing / 2 - stepw - 1 + animateXX, 451 - (ringNumber + 1) * heightRing + 1 - animateYY);
				glVertex2f(centerRod + widthRing / 2 - stepw - 1 + animateXX, 451 - (ringNumber)* heightRing - 1 - animateYY);
				glEnd();
			}
		}
	}

	for (int i = 0; i < sizeof(menuArr) / sizeof(structMenuItem); i++)
	{
		if (menuArr[i].statemenuitem == 1)
			glColor3f(0, 0.443, 0.74);
		else if (menuArr[i].statemenuitem == 2)
			glColor3f(0.004, 0.6, 0.145);
		else 
			glColor3f(0.443, 0.443, 0.443);
		glBegin(GL_QUADS);
			glVertex2f(WinWidth - 10 - widthButton, i * heightButton + 10);
			glVertex2f(WinWidth - 10 - widthButton, (i + 1) * heightButton);
			glVertex2f(WinWidth - 10, (i + 1) * heightButton);
			glVertex2f(WinWidth - 10, i * heightButton + 10);
		glEnd();
		if (menuArr[i].statemenuitem == 1)
			glColor3f(0, 0.67, 0.887);
		else if (menuArr[i].statemenuitem == 2)
			glColor3f(0, 0.925, 0.098);
		else 
			glColor3f(0.67, 0.67, 0.67);
		glBegin(GL_QUADS);
			glVertex2f(WinWidth - 10 - widthButton + 2, i * heightButton + 10 + 2);
			glVertex2f(WinWidth - 10 - widthButton + 2, (i + 1) * heightButton - 2);
			glVertex2f(WinWidth - 10 - 2, (i + 1) * heightButton - 2);
			glVertex2f(WinWidth - 10 - 2, i * heightButton + 10 + 2);
		glEnd();

		if (menuArr[i].statemenuitem == 1)
			glColor3f(0, 0.29, 0.65);
		else if (menuArr[i].statemenuitem == 2)
			glColor3f(0, 0.397, 0.094);
		else
			glColor3f(0.5, 0.5, 0.5);
		glRasterPos2f(WinWidth - 10 - widthButton / 2 - strlen(menuArr[i].content) * 6.4, i * 50 + 35);
		if (i == 5)
			glPrint(menuArr[i].content, stateProgram.RingsCount);
		else 
			glPrint(menuArr[i].content);
	}
	if (stateProgram.timercounter > 0)
		ShowTime();
	if (stateProgram.stategame != stop)
		ShowMovesCount();
	glutSwapBuffers();
}

int** InitData(int _n)
{
	int **_rods;
	_rods = (int**)malloc(3 * sizeof(int*));
	for (int i = 0; i < 3; i++)
	{
		_rods[i] = (int*)malloc(13 * sizeof(int));
		for (int j = 0; j < stateProgram.RingsCount; j++)
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
			glutTimerFunc(20, AnimateRing, _rodnum);
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
			glutTimerFunc(20, AnimateRing, _rodnum);
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
			glutTimerFunc(15, AnimateRing, _rodnum);
			animateRingToYY -= 30;
		}
		else
		{
			SwapRingsAtRods(rodUpNumber, rodDownNumber);
			animatedRodNum = animatedRingNum = rodDownNumber = rodUpNumber = -1;
		}
	}
}

void SwapRingsAtRods(int _rodstart, int _rodfinish)
{
	int tempvalring = Rods[_rodstart][GetIdAnimatedRing(_rodstart)];
	Rods[_rodstart][GetIdAnimatedRing(_rodstart)] = 0;
	for (int i = 0; i < stateProgram.RingsCount; i++)
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
	for (int i = 0; i < stateProgram.RingsCount; i++)
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
	if (((_key == '1') || (_key == '2') || (_key == '3')) && stateProgram.stategame == game)
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
			glutTimerFunc(5, AnimateRing, keynum);
		}
		else if (rodUpNumber != -1) // если есть диск и верхний уже поднят
		{
			if (MoveEnabled(keynum))
			{
				if (rodUpNumber != keynum)
					stateProgram.movescount++; // считаем количество ходов, при этом поднятый и опущенный на этот же стережень диск не считается за ход
				animateRingToXX = 0;
				glutTimerFunc(5, AnimateRing, keynum);
			}
		}
	}
	else if (_key == 13) // нажата клавиша Enter
	{
		if (strcmp(menuArr[stateProgram.nodeToSelectMenuItem].name, "diskcount") == 0)
			ChangeRingsCount();
		else if (strcmp(menuArr[stateProgram.nodeToSelectMenuItem].name, "start") == 0)
		{
			switch (stateProgram.stategame)
			{
			case stop:
				StartNewGame();
				break;
			case game: 
				PauseGame();
				break;
			case pause:
				ResumeGame();
				break;
			}
		}
		else if (strcmp(menuArr[stateProgram.nodeToSelectMenuItem].name, "reset") == 0)
		{
			ResetAllState();
		}
		else if (strcmp(menuArr[stateProgram.nodeToSelectMenuItem].name, "mode") == 0)
		{
			ChangeMode();
		}
		RenderScene();
	}
}

void SKeyboard(int _key, int x, int y)
{
	switch (_key)
	{
	case GLUT_KEY_UP:
		if (stateProgram.nodeToSelectMenuItem < 1)
			return;
		for (int i = stateProgram.nodeToSelectMenuItem - 1; i > -1; i--)
		{
			if (menuArr[i].statemenuitem != 0)
			{
				SetSelectedMenuItem(i);
				break;
			}
		}
		break;
	case GLUT_KEY_DOWN:
		for (int i = stateProgram.nodeToSelectMenuItem + 1; i < sizeof(menuArr) / sizeof(structMenuItem); i++)
		{
			if (menuArr[i].statemenuitem != 0)
			{
				SetSelectedMenuItem(i);
				break;
			}
		}
		//nodeToSelectMenuItem
		break;
	case GLUT_KEY_LEFT:
		break;
	case GLUT_KEY_RIGHT:
		break;
	}
	RenderScene();
}

void ChangeRingsCount()
{
	stateProgram.RingsCount++;
	if (stateProgram.RingsCount > 10)
		stateProgram.RingsCount = 3;
	Rods = InitData(stateProgram.RingsCount);
}

bool MoveEnabled(int _rodnum)
{
	bool _result = true;
	if (rodUpNumber > -1 && rodUpNumber != _rodnum && GetCountRingAtRod(_rodnum) > 0 && GetValAnimatedRing(rodUpNumber) > GetValAnimatedRing(_rodnum))
	{
		_result = false;
	}
	return _result;
}

void SetSelectedMenuItem(int _newselect)
{
	menuArr[stateProgram.nodeToSelectMenuItem].statemenuitem = 1;
	stateProgram.nodeToSelectMenuItem = _newselect;
	menuArr[_newselect].statemenuitem = 2;
}

void timerForGame(int _value)
{
	stateProgram.timercounter++;
	if (stateProgram.stategame == game)
		glutTimerFunc(1000, timerForGame, 10);
	RenderScene();
}
void ShowTime()
{
	glColor3f(0, 0.29, 0.65);
	glRasterPos2f(300, 30);
	glPrint("Время: %02.0f:%02.0f:%02.0f", floor(stateProgram.timercounter / 3600), floor(stateProgram.timercounter / 60), stateProgram.timercounter - floor(stateProgram.timercounter / 60) * 60);
}

void ShowMovesCount()
{
	glColor3f(0, 0.29, 0.65);
	glRasterPos2f(30, 30);
	glPrint("Ходов: %u", stateProgram.movescount);
}

void StartNewGame()
{
	stateProgram.stategame = game;
	stateProgram.timercounter++;
	menuArr[0].statemenuitem = 0; menuArr[2].statemenuitem = 0; menuArr[3].statemenuitem = 0; menuArr[4].statemenuitem = 1; menuArr[5].statemenuitem = 0;
	strcpy_s(menuArr[1].content, "Пауза");
	glutTimerFunc(1000, timerForGame, 10);
}

void PauseGame()
{
	menuArr[0].statemenuitem = 0; menuArr[2].statemenuitem = 1; menuArr[3].statemenuitem = 1; menuArr[4].statemenuitem = 1; menuArr[5].statemenuitem = 0;
	strcpy_s(menuArr[1].content, "Продолжить");
	stateProgram.stategame = pause;
}

void ResumeGame()
{
	menuArr[0].statemenuitem = 0; menuArr[2].statemenuitem = 1; menuArr[3].statemenuitem = 0; menuArr[4].statemenuitem = 1; menuArr[5].statemenuitem = 0;
	strcpy_s(menuArr[1].content, "Пауза");
	glutTimerFunc(1000, timerForGame, 10);
	stateProgram.stategame = game;
}

void ResetAllState()
{
	menuArr[0].statemenuitem = 1; menuArr[2].statemenuitem = 0; menuArr[3].statemenuitem = 1; menuArr[4].statemenuitem = 0; menuArr[5].statemenuitem = 1;
	strcpy_s(menuArr[1].content, "Начать");
	stateProgram.movescount = stateProgram.timercounter = 0;
	stateProgram.stategame = stop;
	Rods = InitData(stateProgram.RingsCount);
}

void ChangeMode()
{
	switch (stateProgram.modegame)
	{
	case manual:
		stateProgram.modegame = automatic;
		strcpy_s(menuArr[0].content, "Демо");
		break;
	case automatic:
		stateProgram.modegame = manual;
		strcpy_s(menuArr[0].content, "Ручками");
		break;
	}
}

GLvoid BuildFont(GLvoid)  // Построение нашего растрового шрифта
{
	HFONT  font;					// Идентификатор фонта
	base = glGenLists(224);			// Выделим место для 224 символов ( НОВОЕ )
		font = CreateFont(-22,		// Высота фонта ( НОВОЕ )
			0,						// Ширина фонта
			0,						// Угол отношения
			0,						// Угол наклона
			FW_BOLD,				// Ширина шрифта
			FALSE,					// Курсив
			FALSE,					// Подчеркивание
			FALSE,					// Перечеркивание
			RUSSIAN_CHARSET,			// Идентификатор набора символов
			OUT_TT_PRECIS,			// Точность вывода
			CLIP_DEFAULT_PRECIS,    // Точность отсечения
			ANTIALIASED_QUALITY,    // Качество вывода
			FF_DONTCARE | DEFAULT_PITCH,  // Семейство и шаг
			"Courier New"			// Имя шрифта
		);      
	SelectObject(wgldc, font);        // Выбрать шрифт, созданный нами ( НОВОЕ )
	wglUseFontBitmaps(wgldc, 32, 224, base); // Построить 224 символов начиная с пробела ( НОВОЕ )
}

GLvoid glPrint(const char *fmt, ...)        // Заказная функция «Печати» GL
{
	char text[256];      // Место для нашей строки
	va_list ap;          // Указатель на список аргументов
	if (fmt == NULL)     // Если нет текста
		return;            // Ничего не делать
	va_start(ap, fmt);           // Разбор строки переменных
	vsprintf_s(text, fmt, ap); // И конвертирование символов в реальные коды
	va_end(ap);                  // Результат помещается в строку
	glPushAttrib(GL_LIST_BIT);      // Протолкнуть биты списка отображения ( НОВОЕ )
	glListBase(base - 32);          // Задать базу символа в 32 ( НОВОЕ )
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);// Текст списками отображения(НОВОЕ)
	glPopAttrib(); // Возврат битов списка отображения ( НОВОЕ )
}

GLvoid KillFont(GLvoid)            // Удаление шрифта
{
	glDeleteLists(base, 224);        // Удаление всех 224 списков отображения ( НОВОЕ )
}

