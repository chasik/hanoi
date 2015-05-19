#include <windows.h>
#include <math.h>
#include <stdio.h>
//#include <conio.h>
#include <math.h>
#include <iostream> // strlen
#include "glut.h"

GLuint  base;
HDC wgldc;
int **Rods;

float WinWidth = 1000, WinHeight = 500;		// размер окна
float widthRing = 200, heightRing = 30;		// базовая ширина и высота диска
float widthButton = 180, heightButton = 50; // размеры кнопок (пунктов меню)
float widthRod = 12, heightRod = 380;		// ширина основы стержня и высота стержня

float animateRingToYY = 0, animateRingToXX = 0, topRod = 13;	// --------------------------------------------
int animatedRodNum = 0, animatedRingNum = 0;					// переменные для анимации (перемещение дисков)
int rodUpNumber = -1, rodDownNumber = -1;						// --------------------------------------------

int iterationCount = 0;

enum stateGame // перечисление статуса игры
{
	stop = 0,
	game = 1,
	pause = 2,
};

enum modeGame // перечисление для хранения режима игры
{
	manual = 0,
	automatic = 1 
};

enum moveDiraction // направление двежения
{
	moveUp = 0,
	moveDown = 1
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

structState stateProgram; // статус состояния программы

struct DoGoArrItem // структура для хранения одного хода
{
	int rodFrom;
	int rodTo;
};
DoGoArrItem *motionArr; // массив расчитанных ходов
int motionArrCount; // расчетное количество ходов по формуле 2 в степени diskcount минус еденица

struct structMenuItem // структура для хранения информации об одном элементе меню
{
	char name[20];		// имя для определения выбранного меню
	char content[20];	// отображаемое название пункта меню (кнопки)
	int statemenuitem;	// состояние кнопки
};


// массив с меню (кнопками)
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

// --------------------------------------------------------------------------------------------------------
// функции для отображения текста (в том числе и кириллицы). Урок NeHe № 13
// --------------------------------------------------------------------------------------------------------
GLvoid BuildFont(GLvoid);
GLvoid KillFont(GLvoid);
GLvoid glPrint(const char *fmt, ...);

void SetSelectedMenuItem(int i); // функция, переводящая пункт в меню в "активное" (выбранное) состояние

int** InitData(int _countdisk); // инициализация двухмерного массива для хранения стержней с дисками на них

void Keyboard(unsigned char _key, int _x, int _y);	// glut - обработка нажатия клавиш (символьные + Enter)
void SKeyboard(int key, int x, int y);				// glut - обработка нажатия клавиш "стрелок"


void AnimateRing(int _rodnum);							// данная функция полностью отвечает за движения диска вверх, в сторону и вниз.

int GetCountRingAtRod(int _rodnum);						// получаем кол-во дисков на указаном стержне
int GetValAnimatedRing(int _rodnum);					// функция возвращает "ширину" диска
int GetIdAnimatedRing(int _rodnum);						// а эта - номер анимированного диска
void SwapRingsAtRods(int _rodstart, int _rodfinish);	// "переброс" диска с одного стержня на другой (номера стержней - входные параметры)
bool MoveEnabled(int _rodnum);							// функция вовзращает truе, если активированный диск можно положить на указанный стержень - иначе false

void timerForGame(int _value); // функция таймера (расчет потраченного времени)
void motionForAutomatic(moveDiraction _diraction);	// данная ф-ия "эмитирует" нажатие клавиш в зависимости от заданного направления. Таким образом реализуем авто режим, не переписывая код,
													// не придумывая еще ф-ию для авто режима
void ShowTime();		// отображение времени
void ShowMovesCount();	// отображение количества ходов
void ChangeRingsCount();// отбработка нажатия кнопки изменения кол-ва дисков

void StartNewGame();	// запуск игры при ручном режиме
void StartNewGameAuto();// запуск игры в автомат. режиме
void PauseGame();		// пауза в игре (как ручн. режим, так и авто)
void ResumeGame();		// возобновление игры после паузы
void ResetAllState();	// сброс
void ChangeMode();		// изменение режима с ручного на авто и обратно

void MoveAutomatic(int _sizestack, int _fromindex, int _toindex); // функция для расчета последовательности ходов. Используется рекурсивный метод решения.

void main(){
	Rods = InitData(stateProgram.RingsCount); // первичная инициализация дисков при запуске программы

	//char fileopt[] = "options.ini";

	// Блок начальной инициализации OpenGl при помощи библиотеки Glut. Создание окна.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WinWidth, WinHeight);
	glutInitWindowPosition(100, 200);
	glutCreateWindow("Ханойская башня. Автор: Костикова Мария");
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	glOrtho(0.0, WinWidth, WinHeight, 0.0, -1.0, 1.0);
	glClearColor(0.85, 0.85, 0.85, 1); // цвет фона делаем серым (R=G=B)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glClearDepth(1.0f);         // Установка буфера глубины
	glEnable(GL_DEPTH_TEST);    // Разрешение теста глубины
	glDepthFunc(GL_LEQUAL);     // Тип теста глубины
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glutKeyboardFunc(Keyboard); // назначение функции обратного вызова для работы с клавиатурой (1,2,3)
	glutSpecialFunc(SKeyboard); // ....... для работы с клавиатурой (движение по меню с помощью стрелок)

	wgldc = wglGetCurrentDC(); // получение контекста устройства (windows API). Нужен для установки шрифта.
	BuildFont(); // построение шрифта

	glutDisplayFunc(RenderScene);	// назначение функции рендеринга (прорисовки) экрана
	RenderScene();					// принудительная первоначальная прорисовка экрана
	glutMainLoop();					// glut - цикл обработки событий, получаемых от интерфейса (окна)

	KillFont(); // удаляем шрифт по завершению
	return;
}

void RenderScene()
{
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
	for (int rodNumber = 0; rodNumber < 3; rodNumber++)		// цикл по количеству стержней
	{
		bool rodMoveEnabled = MoveEnabled(rodNumber);

		int countringatrod = GetCountRingAtRod(rodNumber);
		for (int ringNumber = 0; ringNumber < countringatrod; ringNumber++)
		{
			int ringNum = Rods[rodNumber][ringNumber];		// ширина кольца (от 1 до N, где N - количество дисков)
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

	for (int i = 0; i < sizeof(menuArr) / sizeof(structMenuItem); i++)		// цикл по кол-ву пунктов меню для их отрисовки 
	{
		// в зависимости от состояния используем разный цвет
		if (menuArr[i].statemenuitem == 1)
			glColor3f(0, 0.443, 0.74);
		else if (menuArr[i].statemenuitem == 2)
			glColor3f(0.004, 0.6, 0.145);
		else 
			glColor3f(0.443, 0.443, 0.443);

		glBegin(GL_QUADS); // прорисовка "подложки" под кнопку - для более темного "бортика" по периметру
			glVertex2f(WinWidth - 10 - widthButton, i * heightButton + 10);
			glVertex2f(WinWidth - 10 - widthButton, (i + 1) * heightButton);
			glVertex2f(WinWidth - 10, (i + 1) * heightButton);
			glVertex2f(WinWidth - 10, i * heightButton + 10);
		glEnd();

		glBegin(GL_QUADS);
			// в зависимости от состояния используем разный цвет
			if (menuArr[i].statemenuitem == 1)
				glColor3f(0, 0.77, 0.887);
			else if (menuArr[i].statemenuitem == 2)
				glColor3f(0, 0.97, 0.098);
			else
				glColor3f(0.67, 0.67, 0.67);
			glVertex2f(WinWidth - 10 - 2, i * heightButton + 10 + 2);
			glVertex2f(WinWidth - 10 - widthButton + 2, i * heightButton + 10 + 2);

			// снова разный цвет для состойний кнопки, но это уже для градиента кнопок
			if (menuArr[i].statemenuitem == 1)
				glColor3f(0, 0.45, 0.887);
			else if (menuArr[i].statemenuitem == 2)
				glColor3f(0, 0.5, 0.098);
			else
				glColor3f(0.5, 0.5, 0.5);

			glVertex2f(WinWidth - 10 - widthButton + 2, (i + 1) * heightButton - 2);
			glVertex2f(WinWidth - 10 - 2, (i + 1) * heightButton - 2);
		glEnd();

		// цвет шрифта кнопки в зависимости от состояния
		if (menuArr[i].statemenuitem == 1)
			glColor3f(0, 0.29, 0.65);
		else if (menuArr[i].statemenuitem == 2)
			glColor3f(0, 0.397, 0.094);
		else
			glColor3f(0.4, 0.4, 0.4);
		glRasterPos2f(WinWidth - 10 - widthButton / 2 - strlen(menuArr[i].content) * 6.4, i * 50 + 35);

		if (strcmp(menuArr[i].name, "diskcount") == 0)	// если это кнопка "кол-во дисков" используем немного другой вид подписи, так как передается кол-во дисков
			glPrint(menuArr[i].content, stateProgram.RingsCount);
		else																			// для всех остальных - просто вывод надписи
			glPrint(menuArr[i].content);
	}

	if (stateProgram.timercounter > 0)	// если наш счетчик времени активирован (больше нуля), то отобразим его значение в формате hh:mm:ss
		ShowTime();
	if (stateProgram.stategame != stop)
		ShowMovesCount();				// то же сделаем и для кол-ва ходов, если игра активна

	glutSwapBuffers();					// glut - так как мы используем opengl в режиме с двойным буфером (отрисовка происходит на заднем плане) 
										// - поменяем планы местами и выведем на передний план то что прорисовали до этого момента
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
	if (rodUpNumber < 0) // это анимация ВВЕРХ
	{
		if ((GetIdAnimatedRing(_rodnum) + 1) * heightRing + animateRingToYY < topRod * heightRing)
		{
			glutTimerFunc(30, AnimateRing, _rodnum);
			animateRingToYY += 30;
		}
		else
		{
			rodUpNumber = _rodnum;
			if (stateProgram.modegame == automatic)
				motionForAutomatic(moveDown);
		}
	}
	else if (rodDownNumber < 0) // это анимация В СТОРОНУ
	{
		if ((rodUpNumber < animatedRodNum && rodUpNumber * 250 + 155 + animateRingToXX < animatedRodNum * 250 + 155)
			|| (rodUpNumber > animatedRodNum && rodUpNumber * 250 + 155 + animateRingToXX > animatedRodNum * 250 + 155))
		{
			glutTimerFunc(30, AnimateRing, _rodnum);
			if (abs((animatedRodNum * 250 + 155) - (rodUpNumber * 250 + 155 + animateRingToXX)) > 30)
				animateRingToXX += 30 * (rodUpNumber > animatedRodNum ? -1 : 1);
			else
				animateRingToXX += 5 * (rodUpNumber > animatedRodNum ? -1 : 1);
		}
		else
		{
			rodDownNumber = _rodnum;
			glutTimerFunc(30, AnimateRing, _rodnum);
		}
	}
	else // а это финальная анимация диска ВНИЗ
	{
		if ((GetIdAnimatedRing(_rodnum) * heightRing + animateRingToYY > (GetCountRingAtRod(rodDownNumber) + 1) * heightRing) // опускание на стержень, отличный от стартового
			|| (rodUpNumber == rodDownNumber && (GetIdAnimatedRing(_rodnum) * heightRing + animateRingToYY >= (GetCountRingAtRod(rodDownNumber)) * heightRing))) // возврат на тот же (стартовый) стержень
		{
			glutTimerFunc(30, AnimateRing, _rodnum);
			animateRingToYY -= 30;
		}
		else
		{
			SwapRingsAtRods(rodUpNumber, rodDownNumber);
			animatedRodNum = animatedRingNum = rodDownNumber = rodUpNumber = -1;
			if (stateProgram.modegame == automatic && stateProgram.movescount < motionArrCount)
				motionForAutomatic(moveUp);
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
	if (((_key == '1') || (_key == '2') || (_key == '3')) && stateProgram.stategame == game) // если это нажатая одна из трех цифр при этом статус программы = game (то есть игра начата)
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
	else if (_key == 13) // нажата клавиша Enter - анализируем пункт меню, который вабран на данный момент
	{
		if (strcmp(menuArr[stateProgram.nodeToSelectMenuItem].name, "diskcount") == 0)
			ChangeRingsCount();
		else if (strcmp(menuArr[stateProgram.nodeToSelectMenuItem].name, "start") == 0)
		{
			switch (stateProgram.stategame)
			{
			case stop:
				if (stateProgram.modegame == manual)
					StartNewGame();
				else if (stateProgram.modegame == automatic)
					StartNewGameAuto();
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
		break;
	case GLUT_KEY_LEFT:
		break;
	case GLUT_KEY_RIGHT:
		break;
	}
	RenderScene();
}

void ChangeRingsCount() // смена кол-ва дисков "по кругу" - дойдя до 12 сбарсываем на 3
{
	stateProgram.RingsCount++;
	if (stateProgram.RingsCount > 12)
		stateProgram.RingsCount = 3;
	Rods = InitData(stateProgram.RingsCount); // ... и снова инициализируем начальное состояние дисков на стержнях
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

void motionForAutomatic(moveDiraction _diraction)
{
	int rod;
	if (_diraction == moveUp)
		rod = motionArr[stateProgram.movescount + 1].rodFrom;
	else if (_diraction == moveDown)
		rod = motionArr[stateProgram.movescount + 1].rodTo;
	char chRods;
	if (rod == 0)
		chRods = '1';
	else if (rod == 1)
		chRods = '2';
	else if (rod == 2)
		chRods = '3';
	Keyboard(chRods, 0, 0);
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

void StartNewGameAuto()
{
	strcpy_s(menuArr[1].content, "Пауза");
	menuArr[0].statemenuitem = 0; menuArr[2].statemenuitem = 0; menuArr[3].statemenuitem = 0; menuArr[4].statemenuitem = 0; menuArr[5].statemenuitem = 0;
	iterationCount = 0;
	motionArrCount = pow(2, stateProgram.RingsCount) - 1; // количество ходов по формуле (2 ^ n) - 1
	motionArr = (DoGoArrItem*)malloc(motionArrCount * sizeof(DoGoArrItem));
	MoveAutomatic(stateProgram.RingsCount, 0, 2); // запускаем просчет ходов и сохраняем их в массиве motionArr
	stateProgram.stategame = game;
	motionForAutomatic(moveUp);
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

void MoveAutomatic(int _sizestack, int _fromindex, int _toindex)
{
	if (_sizestack == 0)
		return;

	int freeindex = -1;
	for (int i = 0; i < 3; i++)
	{
		if (_fromindex != i && _toindex != i)
		{
			freeindex = i;
			break;
		}
	}

	MoveAutomatic(_sizestack - 1, _fromindex, freeindex);
	iterationCount++;
	motionArr[iterationCount].rodFrom = _fromindex;
	motionArr[iterationCount].rodTo = _toindex;

	MoveAutomatic(_sizestack - 1, freeindex, _toindex);
}

GLvoid BuildFont(GLvoid)  // Построение нашего растрового шрифта
{
	HFONT  font;						// Идентификатор шрифта
	base = glGenLists(224);				// Выделим место для 224 символов
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

