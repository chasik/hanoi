#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <math.h>
#include <iostream> // strlen
#include "glut.h"
#include "showinfo.h"

#include "glprints.h"

structShowInfo stateShowInfo;

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
	int RingsCount = 5;
	int nodeToSelectMenuItem = 0;
	int timercounter = 0;
	int movescount = -1;
	stateGame stategame = stop;
	modeGame modegame = manual;
	bool isfinish = false;
};

structState stateProgram;	// статус состояния программы

struct DoGoArrItem			// структура для хранения одного хода
{
	int rodFrom;
	int rodTo;
};
DoGoArrItem *motionArr;		// массив расcчитанных ходов
int motionArrCount;			// расчетное количество ходов по формуле 2 в степени diskcount минус еденица

enum stateButton			// перечисление для хранения состояния кнопки
{
	disable = 0,
	enable = 1,
	activate = 2
};

struct structMenuItem		// структура для хранения информации об одном элементе меню
{
	char name[20];			// имя для определения выбранного меню
	char content[20];		// отображаемое название пункта меню (кнопки)
	stateButton statemenuitem;	// состояние кнопки
};

// массив с меню (кнопками)
structMenuItem menuArr[6] = { { "mode", "Ручками", activate }
							, { "start", "Начать", enable }
							, { "reset", "Сброс", disable }
							, { "load", "Загрузить", enable }
							, { "save", "Сохранить", disable }
							, { "diskcount", "Дисков %u", enable }
};

// --------------------------------------------------------------------------------------------------------------
// Функция обратного вызова для рисования сцены
// --------------------------------------------------------------------------------------------------------------
void RenderScene();

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
void ShowTime(int _x = 20, int _y = 48);		// отображение времени
void ShowMovesCount(int _x = 20, int _y = 26);	// отображение количества ходов
void ChangeRingsCount();// отбработка нажатия кнопки изменения кол-ва дисков

void StartNewGame();	// запуск игры при ручном режиме
void StartNewGameAuto();// запуск игры в автомат. режиме
void PauseGame();		// пауза в игре (как ручн. режим, так и авто)
void ResumeGame();		// возобновление игры после паузы
void ResetAllState();	// сброс
void ChangeMode();		// изменение режима с ручного на авто и обратно

void MoveAutomatic(int _sizestack, int _fromindex, int _toindex); // функция для расчета последовательности ходов. Используется рекурсивный метод решения.

void LoadGame();						// функция загрузки ранее сохраненного состояния игры
void SaveGame();						// сохранение состояния игры

void ShowFinishWindow();				// вывод результатов при успешном завершении игры

void main(){
	Rods = InitData(stateProgram.RingsCount); // первичная инициализация дисков при запуске программы

	// Блок начальной инициализации OpenGl при помощи библиотеки Glut. Создание окна.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WinWidth, WinHeight);
	glutInitWindowPosition(100, 200);
	glutCreateWindow("Ханойская башня. Автор: Костикова Мария");
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	glOrtho(0.0, WinWidth, WinHeight, 0.0, -1.0, 1.0);
	glClearColor(0.85, 0.85, 0.85, 1); // цвет фона делаем серым (R=G=B)

	glClearDepth(1.0f);				// Установка буфера глубины
	glEnable(GL_DEPTH_TEST);		// Разрешение теста глубины
	glDepthFunc(GL_LEQUAL);			// Тип теста глубины
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glutKeyboardFunc(Keyboard);		// назначение функции обратного вызова для работы с клавиатурой (1,2,3)
	glutSpecialFunc(SKeyboard);		// ....... для работы с клавиатурой (движение по меню с помощью стрелок)

	BuildFont(22);					// построение шрифта

	glutDisplayFunc(RenderScene);	// назначение функции рендеринга (прорисовки) экрана
	RenderScene();					// принудительная первоначальная прорисовка экрана
	glutMainLoop();					// glut - цикл обработки событий, получаемых от интерфейса (окна)

	KillFont();						// удаляем шрифт по завершению
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
		glColor3f(0, 0.2, 0.9);
		glRasterPos2f(i * 250 - 100, 490);
		glPrint("%u", i);
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
	{
		glColor3f(0, 0.29, 0.65);
		ShowTime();
	}
	if (stateProgram.movescount > -1)
	{
		glColor3f(0, 0.29, 0.65);
		ShowMovesCount();				// то же сделаем и для кол-ва ходов, если игра активна
	}

	if (stateShowInfo.timer > 0 && stateShowInfo.timer < stateShowInfo.timerlimit)
		ShowInfo(&stateShowInfo);
	if (stateProgram.isfinish == true)
		ShowFinishWindow();
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
		for (int j = 0; j < 13; j++)
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
			// Завершение анимации диска
			SwapRingsAtRods(rodUpNumber, rodDownNumber);
			animatedRodNum = animatedRingNum = rodDownNumber = rodUpNumber = -1;
			if (stateProgram.modegame == automatic && stateProgram.movescount < motionArrCount)
				motionForAutomatic(moveUp);
			if (GetCountRingAtRod(0) == 0 && GetCountRingAtRod(1) == 0 && GetCountRingAtRod(2) == stateProgram.RingsCount)
			{
				stateProgram.isfinish = true;
				RenderScene();
			}
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
				animatedRodNum = animatedRingNum = rodDownNumber = rodUpNumber = -1;
				stateProgram.isfinish = false;
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
		else if (strcmp(menuArr[stateProgram.nodeToSelectMenuItem].name, "load") == 0)
		{
			LoadGame();
		}
		else if (strcmp(menuArr[stateProgram.nodeToSelectMenuItem].name, "save") == 0)
		{
			SaveGame();
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
	menuArr[stateProgram.nodeToSelectMenuItem].statemenuitem = enable;
	stateProgram.nodeToSelectMenuItem = _newselect;
	menuArr[_newselect].statemenuitem = activate;
}

void timerForGame(int _value)
{
	if (stateProgram.stategame == game && !stateProgram.isfinish)
	{
		stateProgram.timercounter++;
		glutTimerFunc(1000, timerForGame, 10);
	}
	RenderScene();
}

void motionForAutomatic(moveDiraction _diraction)
{
	int rod;
	if (_diraction == moveUp)
		rod = motionArr[stateProgram.movescount + 1].rodFrom;
	else if (_diraction == moveDown)
		rod = motionArr[stateProgram.movescount + 1].rodTo;
	char chRods = '0';
	if (rod == 0)
		chRods = '1';
	else if (rod == 1)
		chRods = '2';
	else if (rod == 2)
		chRods = '3';
	if (chRods != '0')
	Keyboard(chRods, 0, 0);
}

void ShowTime(int _x, int _y)
{
	glRasterPos2f(_x, _y);
	glPrint("Время: %02.0f:%02.0f:%02.0f", floor(stateProgram.timercounter / 3600), floor(stateProgram.timercounter / 60), stateProgram.timercounter - floor(stateProgram.timercounter / 60) * 60);
}

void ShowMovesCount(int _x, int _y)
{
	glRasterPos2f(_x, _y);
	glPrint("Ходов: %u", stateProgram.movescount);
}

void StartNewGame()
{
	stateProgram.stategame = game;
	stateProgram.timercounter++;
	menuArr[0].statemenuitem = disable; menuArr[2].statemenuitem = disable; menuArr[3].statemenuitem = disable; menuArr[4].statemenuitem = enable; menuArr[5].statemenuitem = disable;
	strcpy_s(menuArr[1].content, "Пауза");
	glutTimerFunc(1000, timerForGame, 10);
}

void StartNewGameAuto()
{
	menuArr[2].statemenuitem = activate; menuArr[0].statemenuitem = disable; menuArr[1].statemenuitem = disable; menuArr[3].statemenuitem = disable; menuArr[4].statemenuitem = disable; menuArr[5].statemenuitem = disable;
	stateProgram.nodeToSelectMenuItem = 2;
	iterationCount = 0;
	motionArrCount = pow(2, stateProgram.RingsCount) - 1; // количество ходов по формуле (2 ^ n) - 1
	motionArr = (DoGoArrItem*)malloc(motionArrCount * sizeof(DoGoArrItem));
	stateProgram.movescount = 0;
	MoveAutomatic(stateProgram.RingsCount, 0, 2); // запускаем просчет ходов и сохраняем их в массиве motionArr
	stateProgram.stategame = game;
	motionForAutomatic(moveUp);
	glutTimerFunc(1000, timerForGame, 10);
}

void PauseGame()
{
	stateProgram.nodeToSelectMenuItem = 1;
	menuArr[1].statemenuitem = activate;
	menuArr[0].statemenuitem = disable; menuArr[2].statemenuitem = enable; menuArr[3].statemenuitem = disable; menuArr[5].statemenuitem = disable;
	menuArr[4].statemenuitem = stateProgram.modegame == manual ? enable : disable;
	strcpy_s(menuArr[1].content, "Продолжить");
	stateProgram.stategame = pause;
}

void ResumeGame()
{
	menuArr[0].statemenuitem = disable; menuArr[2].statemenuitem = disable; menuArr[3].statemenuitem = disable; menuArr[4].statemenuitem = enable; menuArr[5].statemenuitem = disable;
	strcpy_s(menuArr[1].content, "Пауза");
	if (stateProgram.modegame == manual)
		glutTimerFunc(1000, timerForGame, 10);
	stateProgram.stategame = game;
}

void ResetAllState()
{
	stateProgram.isfinish = false;
	animatedRodNum = animatedRingNum = rodDownNumber = rodUpNumber = -1;
	stateProgram.nodeToSelectMenuItem = 0;
	menuArr[0].statemenuitem = activate;
	menuArr[1].statemenuitem = enable;
	menuArr[2].statemenuitem = disable; menuArr[3].statemenuitem = enable; menuArr[4].statemenuitem = disable; menuArr[5].statemenuitem = enable;
	strcpy_s(menuArr[1].content, "Начать");
	stateProgram.movescount = -1;
	stateProgram.timercounter = 0;
	stateProgram.stategame = stop;
	stateProgram.modegame = manual;
	ChangeMode();															// изменим подпись кнопки выбора режима (установим ее в "ручками")
	Rods = InitData(stateProgram.RingsCount);
}

void ChangeMode()
{
	switch (stateProgram.modegame)
	{
	case manual:
		stateProgram.modegame = automatic;
		strcpy_s(menuArr[0].content, "Авто");
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

void LoadGame()
{
	Rods = InitData(0);
	FILE  *inifile;
	char readstr[255], cutstr[10];
	int rod = -1, ring, ringval;
	int ringsall = 0;
	if (!fopen_s(&inifile, "saved.ini", "r"))
	{
		while (!feof(inifile))
		{
			fgets(readstr, 255, inifile);
			if (strstr(readstr, "movescount") != 0)
			{
				strncpy_s(cutstr, readstr + 11, 10);
				stateProgram.movescount = atoi(cutstr);
			}
			else if (strstr(readstr, "timercounter") != 0)
			{
				strncpy_s(cutstr, readstr + 13, 10);
				stateProgram.timercounter = atoi(cutstr);
			}
			else if (strstr(readstr, "ringscount") != 0)
			{
				strncpy_s(cutstr, readstr + 11, 10);
				stateProgram.RingsCount = atoi(cutstr);
			}
			else if (strstr(readstr, "rod") != 0)
			{
				strncpy_s(cutstr, readstr + 3, 10);
				rod = atoi(cutstr);
			}
			else if (strstr(readstr, "ring") != 0)
			{
				strncpy_s(cutstr, readstr + 4, 10);
				ring = atoi(cutstr);
				strncpy_s(cutstr, readstr + 6, 10);
				ringval = atoi(cutstr);
				Rods[rod - 1][ring] = ringval;
			}
		}
		fclose(inifile);
		ShowInfoInit("Данные загружены!!!", &stateShowInfo, green);
		RenderScene();
		stateProgram.modegame = manual; // сразу сбросили в ручной режим, даже если до загрузки был авто
		PauseGame();
	}
	else 
	{
		ShowInfoInit("Ошибка чтения!!!", &stateShowInfo, red);
	}
}

void SaveGame()
{
	FILE  *inifile;
	if (!fopen_s(&inifile, "saved.ini", "w"))
	{
		fprintf_s(inifile, "movescount=%d\n", stateProgram.movescount);
		fprintf_s(inifile, "timercounter=%d\n", stateProgram.timercounter);
		fprintf_s(inifile, "ringscount=%d\n", stateProgram.RingsCount);
		fprintf_s(inifile, "rod1=%d\n", GetCountRingAtRod(0));
		for (int i = 0; i < GetCountRingAtRod(0); i++)
			fprintf_s(inifile, "ring%d=%d\n", i, Rods[0][i]);

		fprintf_s(inifile, "rod2=%d\n", GetCountRingAtRod(1));
		for (int i = 0; i < GetCountRingAtRod(1); i++)
			fprintf_s(inifile, "ring%d=%d\n", i, Rods[1][i]);

		fprintf_s(inifile, "rod3=%d\n", GetCountRingAtRod(2));
		for (int i = 0; i < GetCountRingAtRod(2); i++)
			fprintf_s(inifile, "ring%d=%d\n", i, Rods[2][i]);

		fclose(inifile);
		ShowInfoInit("Игра сохранена!!!", &stateShowInfo, green);
	}
	else
	{
		ShowInfoInit("Ошибка сохранения!!!", &stateShowInfo, red);
	}
}

void ShowFinishWindow()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
		glColor4f(0.1, 0.1, 1, 0.9);
		glVertex2f(5, 5);
		glVertex2f(5, 495);
		glVertex2f(805, 495);
	glVertex2f(805, 5);
	glEnd();
	glDisable(GL_BLEND);
	BuildFont(40);
	glColor3f(1, 0.1, 0.1);
	glRasterPos2f(30, 40);
	glPrint("Игра завершена!");
	BuildFont(22);
	ShowMovesCount(30, 80);
	ShowTime(30, 120);
	// оставляем доступной только кнопку сброса
	menuArr[2].statemenuitem = activate; menuArr[0].statemenuitem = disable; menuArr[1].statemenuitem = disable; menuArr[3].statemenuitem = disable; menuArr[4].statemenuitem = disable; menuArr[5].statemenuitem = disable;
	stateProgram.nodeToSelectMenuItem = 2;
}
