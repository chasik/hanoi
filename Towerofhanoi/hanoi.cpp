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

float WinWidth = 1000, WinHeight = 500;		// ������ ����
float widthRing = 200, heightRing = 30;		// ������� ������ � ������ �����
float widthButton = 180, heightButton = 50; // ������� ������ (������� ����)
float widthRod = 12, heightRod = 380;		// ������ ������ ������� � ������ �������

float animateRingToYY = 0, animateRingToXX = 0, topRod = 13;	// --------------------------------------------
int animatedRodNum = 0, animatedRingNum = 0;					// ���������� ��� �������� (����������� ������)
int rodUpNumber = -1, rodDownNumber = -1;						// --------------------------------------------

int iterationCount = 0;

enum stateGame // ������������ ������� ����
{
	stop = 0,
	game = 1,
	pause = 2,
};

enum modeGame // ������������ ��� �������� ������ ����
{
	manual = 0,
	automatic = 1 
};

enum moveDiraction // ����������� ��������
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

structState stateProgram;	// ������ ��������� ���������

struct DoGoArrItem			// ��������� ��� �������� ������ ����
{
	int rodFrom;
	int rodTo;
};
DoGoArrItem *motionArr;		// ������ ���c�������� �����
int motionArrCount;			// ��������� ���������� ����� �� ������� 2 � ������� diskcount ����� �������

enum stateButton			// ������������ ��� �������� ��������� ������
{
	disable = 0,
	enable = 1,
	activate = 2
};

struct structMenuItem		// ��������� ��� �������� ���������� �� ����� �������� ����
{
	char name[20];			// ��� ��� ����������� ���������� ����
	char content[20];		// ������������ �������� ������ ���� (������)
	stateButton statemenuitem;	// ��������� ������
};

// ������ � ���� (��������)
structMenuItem menuArr[6] = { { "mode", "�������", activate }
							, { "start", "������", enable }
							, { "reset", "�����", disable }
							, { "load", "���������", enable }
							, { "save", "���������", disable }
							, { "diskcount", "������ %u", enable }
};

// --------------------------------------------------------------------------------------------------------------
// ������� ��������� ������ ��� ��������� �����
// --------------------------------------------------------------------------------------------------------------
void RenderScene();

void SetSelectedMenuItem(int i); // �������, ����������� ����� � ���� � "��������" (���������) ���������

int** InitData(int _countdisk); // ������������� ����������� ������� ��� �������� �������� � ������� �� ���

void Keyboard(unsigned char _key, int _x, int _y);	// glut - ��������� ������� ������ (���������� + Enter)
void SKeyboard(int key, int x, int y);				// glut - ��������� ������� ������ "�������"

void AnimateRing(int _rodnum);							// ������ ������� ��������� �������� �� �������� ����� �����, � ������� � ����.

int GetCountRingAtRod(int _rodnum);						// �������� ���-�� ������ �� �������� �������
int GetValAnimatedRing(int _rodnum);					// ������� ���������� "������" �����
int GetIdAnimatedRing(int _rodnum);						// � ��� - ����� �������������� �����
void SwapRingsAtRods(int _rodstart, int _rodfinish);	// "��������" ����� � ������ ������� �� ������ (������ �������� - ������� ���������)
bool MoveEnabled(int _rodnum);							// ������� ���������� tru�, ���� �������������� ���� ����� �������� �� ��������� �������� - ����� false

void timerForGame(int _value); // ������� ������� (������ ������������ �������)
void motionForAutomatic(moveDiraction _diraction);	// ������ �-�� "���������" ������� ������ � ����������� �� ��������� �����������. ����� ������� ��������� ���� �����, �� ����������� ���,
													// �� ���������� ��� �-�� ��� ���� ������
void ShowTime(int _x = 20, int _y = 48);		// ����������� �������
void ShowMovesCount(int _x = 20, int _y = 26);	// ����������� ���������� �����
void ChangeRingsCount();// ���������� ������� ������ ��������� ���-�� ������

void StartNewGame();	// ������ ���� ��� ������ ������
void StartNewGameAuto();// ������ ���� � �������. ������
void PauseGame();		// ����� � ���� (��� ����. �����, ��� � ����)
void ResumeGame();		// ������������� ���� ����� �����
void ResetAllState();	// �����
void ChangeMode();		// ��������� ������ � ������� �� ���� � �������

void MoveAutomatic(int _sizestack, int _fromindex, int _toindex); // ������� ��� ������� ������������������ �����. ������������ ����������� ����� �������.

void LoadGame();						// ������� �������� ����� ������������ ��������� ����
void SaveGame();						// ���������� ��������� ����

void ShowFinishWindow();				// ����� ����������� ��� �������� ���������� ����

void main(){
	Rods = InitData(stateProgram.RingsCount); // ��������� ������������� ������ ��� ������� ���������

	// ���� ��������� ������������� OpenGl ��� ������ ���������� Glut. �������� ����.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WinWidth, WinHeight);
	glutInitWindowPosition(100, 200);
	glutCreateWindow("��������� �����. �����: ��������� �����");
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	glOrtho(0.0, WinWidth, WinHeight, 0.0, -1.0, 1.0);
	glClearColor(0.85, 0.85, 0.85, 1); // ���� ���� ������ ����� (R=G=B)

	glClearDepth(1.0f);				// ��������� ������ �������
	glEnable(GL_DEPTH_TEST);		// ���������� ����� �������
	glDepthFunc(GL_LEQUAL);			// ��� ����� �������
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glutKeyboardFunc(Keyboard);		// ���������� ������� ��������� ������ ��� ������ � ����������� (1,2,3)
	glutSpecialFunc(SKeyboard);		// ....... ��� ������ � ����������� (�������� �� ���� � ������� �������)

	BuildFont(22);					// ���������� ������

	glutDisplayFunc(RenderScene);	// ���������� ������� ���������� (����������) ������
	RenderScene();					// �������������� �������������� ���������� ������
	glutMainLoop();					// glut - ���� ��������� �������, ���������� �� ���������� (����)

	KillFont();						// ������� ����� �� ����������
	return;
}

void RenderScene()
{
	float maxWidthRing = 220, minWidthRing = 120;
	float stepWidthRing = (maxWidthRing - minWidthRing) / stateProgram.RingsCount;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // ������� ����� � ������ �������

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
	for (int rodNumber = 0; rodNumber < 3; rodNumber++)		// ���� �� ���������� ��������
	{
		bool rodMoveEnabled = MoveEnabled(rodNumber);

		int countringatrod = GetCountRingAtRod(rodNumber);
		for (int ringNumber = 0; ringNumber < countringatrod; ringNumber++)
		{
			int ringNum = Rods[rodNumber][ringNumber];		// ������ ������ (�� 1 �� N, ��� N - ���������� ������)
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

	for (int i = 0; i < sizeof(menuArr) / sizeof(structMenuItem); i++)		// ���� �� ���-�� ������� ���� ��� �� ��������� 
	{
		// � ����������� �� ��������� ���������� ������ ����
		if (menuArr[i].statemenuitem == 1)
			glColor3f(0, 0.443, 0.74);
		else if (menuArr[i].statemenuitem == 2)
			glColor3f(0.004, 0.6, 0.145);
		else 
			glColor3f(0.443, 0.443, 0.443);

		glBegin(GL_QUADS); // ���������� "��������" ��� ������ - ��� ����� ������� "�������" �� ���������
			glVertex2f(WinWidth - 10 - widthButton, i * heightButton + 10);
			glVertex2f(WinWidth - 10 - widthButton, (i + 1) * heightButton);
			glVertex2f(WinWidth - 10, (i + 1) * heightButton);
			glVertex2f(WinWidth - 10, i * heightButton + 10);
		glEnd();

		glBegin(GL_QUADS);
			// � ����������� �� ��������� ���������� ������ ����
			if (menuArr[i].statemenuitem == 1)
				glColor3f(0, 0.77, 0.887);
			else if (menuArr[i].statemenuitem == 2)
				glColor3f(0, 0.97, 0.098);
			else
				glColor3f(0.67, 0.67, 0.67);
			glVertex2f(WinWidth - 10 - 2, i * heightButton + 10 + 2);
			glVertex2f(WinWidth - 10 - widthButton + 2, i * heightButton + 10 + 2);

			// ����� ������ ���� ��� ��������� ������, �� ��� ��� ��� ��������� ������
			if (menuArr[i].statemenuitem == 1)
				glColor3f(0, 0.45, 0.887);
			else if (menuArr[i].statemenuitem == 2)
				glColor3f(0, 0.5, 0.098);
			else
				glColor3f(0.5, 0.5, 0.5);

			glVertex2f(WinWidth - 10 - widthButton + 2, (i + 1) * heightButton - 2);
			glVertex2f(WinWidth - 10 - 2, (i + 1) * heightButton - 2);
		glEnd();

		// ���� ������ ������ � ����������� �� ���������
		if (menuArr[i].statemenuitem == 1)
			glColor3f(0, 0.29, 0.65);
		else if (menuArr[i].statemenuitem == 2)
			glColor3f(0, 0.397, 0.094);
		else
			glColor3f(0.4, 0.4, 0.4);
		glRasterPos2f(WinWidth - 10 - widthButton / 2 - strlen(menuArr[i].content) * 6.4, i * 50 + 35);

		if (strcmp(menuArr[i].name, "diskcount") == 0)	// ���� ��� ������ "���-�� ������" ���������� ������� ������ ��� �������, ��� ��� ���������� ���-�� ������
			glPrint(menuArr[i].content, stateProgram.RingsCount);
		else																			// ��� ���� ��������� - ������ ����� �������
			glPrint(menuArr[i].content);
	}

	if (stateProgram.timercounter > 0)	// ���� ��� ������� ������� ����������� (������ ����), �� ��������� ��� �������� � ������� hh:mm:ss
	{
		glColor3f(0, 0.29, 0.65);
		ShowTime();
	}
	if (stateProgram.movescount > -1)
	{
		glColor3f(0, 0.29, 0.65);
		ShowMovesCount();				// �� �� ������� � ��� ���-�� �����, ���� ���� �������
	}

	if (stateShowInfo.timer > 0 && stateShowInfo.timer < stateShowInfo.timerlimit)
		ShowInfo(&stateShowInfo);
	if (stateProgram.isfinish == true)
		ShowFinishWindow();
	glutSwapBuffers();					// glut - ��� ��� �� ���������� opengl � ������ � ������� ������� (��������� ���������� �� ������ �����) 
										// - �������� ����� ������� � ������� �� �������� ���� �� ��� ����������� �� ����� �������
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
	if (rodUpNumber < 0) // ��� �������� �����
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
	else if (rodDownNumber < 0) // ��� �������� � �������
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
	else // � ��� ��������� �������� ����� ����
	{
		if ((GetIdAnimatedRing(_rodnum) * heightRing + animateRingToYY > (GetCountRingAtRod(rodDownNumber) + 1) * heightRing) // ��������� �� ��������, �������� �� ����������
			|| (rodUpNumber == rodDownNumber && (GetIdAnimatedRing(_rodnum) * heightRing + animateRingToYY >= (GetCountRingAtRod(rodDownNumber)) * heightRing))) // ������� �� ��� �� (���������) ��������
		{
			glutTimerFunc(30, AnimateRing, _rodnum);
			animateRingToYY -= 30;
		}
		else
		{
			// ���������� �������� �����
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
	if (((_key == '1') || (_key == '2') || (_key == '3')) && stateProgram.stategame == game) // ���� ��� ������� ���� �� ���� ���� ��� ���� ������ ��������� = game (�� ���� ���� ������)
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
		if (rodUpNumber == -1) // ���� ���� ���� � �� ���� ���� ��� �� ������
		{
			animatedRingNum = GetValAnimatedRing(keynum);
			animateRingToYY = 0;
			animateRingToXX = 0;
			glutTimerFunc(5, AnimateRing, keynum);
		}
		else if (rodUpNumber != -1) // ���� ���� ���� � ������� ��� ������
		{
			if (MoveEnabled(keynum))
			{
				if (rodUpNumber != keynum)
					stateProgram.movescount++; // ������� ���������� �����, ��� ���� �������� � ��������� �� ���� �� ��������� ���� �� ��������� �� ���
				animateRingToXX = 0;
				glutTimerFunc(5, AnimateRing, keynum);
			}
		}
	}
	else if (_key == 13) // ������ ������� Enter - ����������� ����� ����, ������� ������ �� ������ ������
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

void ChangeRingsCount() // ����� ���-�� ������ "�� �����" - ����� �� 12 ���������� �� 3
{
	stateProgram.RingsCount++;
	if (stateProgram.RingsCount > 12)
		stateProgram.RingsCount = 3;
	Rods = InitData(stateProgram.RingsCount); // ... � ����� �������������� ��������� ��������� ������ �� ��������
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
	glPrint("�����: %02.0f:%02.0f:%02.0f", floor(stateProgram.timercounter / 3600), floor(stateProgram.timercounter / 60), stateProgram.timercounter - floor(stateProgram.timercounter / 60) * 60);
}

void ShowMovesCount(int _x, int _y)
{
	glRasterPos2f(_x, _y);
	glPrint("�����: %u", stateProgram.movescount);
}

void StartNewGame()
{
	stateProgram.stategame = game;
	stateProgram.timercounter++;
	menuArr[0].statemenuitem = disable; menuArr[2].statemenuitem = disable; menuArr[3].statemenuitem = disable; menuArr[4].statemenuitem = enable; menuArr[5].statemenuitem = disable;
	strcpy_s(menuArr[1].content, "�����");
	glutTimerFunc(1000, timerForGame, 10);
}

void StartNewGameAuto()
{
	menuArr[2].statemenuitem = activate; menuArr[0].statemenuitem = disable; menuArr[1].statemenuitem = disable; menuArr[3].statemenuitem = disable; menuArr[4].statemenuitem = disable; menuArr[5].statemenuitem = disable;
	stateProgram.nodeToSelectMenuItem = 2;
	iterationCount = 0;
	motionArrCount = pow(2, stateProgram.RingsCount) - 1; // ���������� ����� �� ������� (2 ^ n) - 1
	motionArr = (DoGoArrItem*)malloc(motionArrCount * sizeof(DoGoArrItem));
	stateProgram.movescount = 0;
	MoveAutomatic(stateProgram.RingsCount, 0, 2); // ��������� ������� ����� � ��������� �� � ������� motionArr
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
	strcpy_s(menuArr[1].content, "����������");
	stateProgram.stategame = pause;
}

void ResumeGame()
{
	menuArr[0].statemenuitem = disable; menuArr[2].statemenuitem = disable; menuArr[3].statemenuitem = disable; menuArr[4].statemenuitem = enable; menuArr[5].statemenuitem = disable;
	strcpy_s(menuArr[1].content, "�����");
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
	strcpy_s(menuArr[1].content, "������");
	stateProgram.movescount = -1;
	stateProgram.timercounter = 0;
	stateProgram.stategame = stop;
	stateProgram.modegame = manual;
	ChangeMode();															// ������� ������� ������ ������ ������ (��������� �� � "�������")
	Rods = InitData(stateProgram.RingsCount);
}

void ChangeMode()
{
	switch (stateProgram.modegame)
	{
	case manual:
		stateProgram.modegame = automatic;
		strcpy_s(menuArr[0].content, "����");
		break;
	case automatic:
		stateProgram.modegame = manual;
		strcpy_s(menuArr[0].content, "�������");
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
		ShowInfoInit("������ ���������!!!", &stateShowInfo, green);
		RenderScene();
		stateProgram.modegame = manual; // ����� �������� � ������ �����, ���� ���� �� �������� ��� ����
		PauseGame();
	}
	else 
	{
		ShowInfoInit("������ ������!!!", &stateShowInfo, red);
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
		ShowInfoInit("���� ���������!!!", &stateShowInfo, green);
	}
	else
	{
		ShowInfoInit("������ ����������!!!", &stateShowInfo, red);
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
	glPrint("���� ���������!");
	BuildFont(22);
	ShowMovesCount(30, 80);
	ShowTime(30, 120);
	// ��������� ��������� ������ ������ ������
	menuArr[2].statemenuitem = activate; menuArr[0].statemenuitem = disable; menuArr[1].statemenuitem = disable; menuArr[3].statemenuitem = disable; menuArr[4].statemenuitem = disable; menuArr[5].statemenuitem = disable;
	stateProgram.nodeToSelectMenuItem = 2;
}
