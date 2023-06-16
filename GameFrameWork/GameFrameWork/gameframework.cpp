/*
	headX,headY表示蛇头
	0表示蛇头，o表示蛇身,@表示fruit，#表示墙
	tail中tail[0]、tail[1]存第一个o，tail[n]存放第n个o

	按键说明：
	wasd和上下左右键控制蛇移动方向
	j暂停，k取消暂停，x结束游戏
	enter键切换全角半角字符显示
	按空格键暂停游戏，按其他键继续游戏
*/
#define _CRT_SECURE_NO_DEPRECATE

#include<iostream>
#include<Windows.h>
#include<time.h>
#include<conio.h>

#define STAGE_WIDTH 20
#define STAGE_HEIGHT 20
#define WINDOW_WIDTH 80
#define WINDOW_HEIGHT 25
#define CORNER_X 5	//游戏左上角坐标
#define CORNER_Y 5
#define THICHKNESS 1	//墙厚度
#define MAXLENGTH 100
#define COLOR_WALL 0X06
#define COLOR_TEXT 0X0F
#define COLOR_TEXT_GAMEOVER 0XEC
#define COLOR_SCORE 0X0C
#define COLOR_FRUIT 0X04
#define COLOR_SNAKE_HEAD 0X09
#define COLOR_SNAKE_BODY 0X0A

#define DETA_X 1
#define DETA_Y 1
#define EDGE_THICKNESS 1

#define DIFFICULTY_FACTOR 50

using namespace std;

HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
HANDLE hOutPut, hOutBuf;
COORD coord = { 0,0 };
DWORD bytes = 0;
DWORD BufferSwapFlag = false;

bool gameOver;
bool fruitFlash;
bool isFullWidth;
bool isPause;
bool updateMap;

const int width = STAGE_WIDTH;
const int height = STAGE_HEIGHT;
char ScreenData[STAGE_WIDTH + 5][STAGE_HEIGHT + 5];

int headX, headY, fruitX, fruitY, score;
enum eDirection{ STOP = 0 ,LEFT ,RIGHT, UP, DOWN };
eDirection dir;
int tailX[MAXLENGTH],tailY[MAXLENGTH];
int ntail;

//游戏帧率管理
const int FRAMES_PER_SECOND = 25;	//恒定帧率
const int SKIP_TICKS = 1000 / FRAMES_PER_SECOND;
DWORD next_Game_Tick = GetTickCount();
int sleep_Time = 0;


void Initial()
{

	SetConsoleTitleA("Console_贪吃蛇*全角字符版*");
	COORD dSize = { WINDOW_WIDTH,WINDOW_HEIGHT };
	SetConsoleScreenBufferSize(h, dSize);	//设置窗口缓冲区大小
	CONSOLE_CURSOR_INFO _cursor = { 1,false };	//设置光标大小，隐藏光标
	SetConsoleCursorInfo(h, &_cursor);

	fruitFlash = false;
	gameOver = false;
	isFullWidth = true;
	isPause = false;

	//蛇头、fruit初始化
	srand(time(NULL));
	dir = STOP;
	headX = width / 2;
	headY = height / 2;
	fruitX = rand() % width;
	fruitY = rand() % height;
	score = 0;

	//蛇身初始化
	ntail = 1;
	for (int i = 0; i < MAXLENGTH; i++)
	{
		tailX[i] = 0;
		tailY[i] = 0;
	}

	//-------局部更新
	//SetConsoleTitleA("Console_贪吃蛇");
	//COORD dSize = { WINDOW_WIDTH,WINDOW_HEIGHT };
	//SetConsoleScreenBufferSize(h, dSize);	//设置窗口缓冲区大小
	//CONSOLE_CURSOR_INFO _cursor = { 1,false };	//设置光标大小，隐藏光标
	//SetConsoleCursorInfo(h, &_cursor);
	//fruitFlash = false;

	//-------双缓冲显示
	
	hOutBuf = CreateConsoleScreenBuffer(
		GENERIC_WRITE,		//定义进程可以往缓冲区写数据
		FILE_SHARE_WRITE,	//定义缓冲区可共享写权限
		NULL,
		CONSOLE_TEXTMODE_BUFFER,
		NULL
	);
	hOutPut = CreateConsoleScreenBuffer(
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CONSOLE_TEXTMODE_BUFFER,
		NULL
	);
	//隐藏两个缓冲区的光标
	CONSOLE_CURSOR_INFO cci;
	cci.bVisible = 0;
	cci.dwSize = 1;
	SetConsoleCursorInfo(hOutPut, &cci);
	SetConsoleCursorInfo(hOutBuf, &cci);

}


//设置光标位置
void setPos(int X, int Y)
{
	COORD pos;
	if (isFullWidth)
	{
		pos.X = CORNER_X * 2 + 2 * X + 2;	//全角字符占用两个半角字符的位置，故步进是原先的两倍
		pos.Y = CORNER_Y + Y + 2;
	}
	else
	{
		pos.X = CORNER_X + X + 2;
		pos.Y = CORNER_Y + Y + 2;
	}

	/*pos.X = X + DETA_X;
	pos.Y = Y + DETA_Y;*/
	SetConsoleCursorPosition(h, pos);
}

//--------------------双缓冲显示
void Draw_DoubleBuffer()
{
	int i, j;
	int currentLine = 0;

	for ( j = 0; j < width + 2; j++)
	{
		ScreenData[currentLine][j] = '#';
	}
	currentLine++;

	for ( i = 0; i < height; i++)
	{
		for ( j = 0; j < width; j++)
		{
			if (j == 0)
				ScreenData[currentLine + i][j] = '#';	
			else if (i == fruitY && j == fruitX)
			{
				ScreenData[currentLine + i][j] = '@';
			}
			else if (i == headY && j == headX)
			{
				ScreenData[currentLine + i][j] = 'O';
			}
			else
			{

				bool flagprint = false;
				for (int k = 1; k < ntail; k++)
				{
					if (tailX[k] == j && tailY[k] == i)
					{
						ScreenData[currentLine + i][j] = 'o';
						flagprint = true;
						break;	//避免蛇身重叠
					}
				}

				
				if (!flagprint) ScreenData[currentLine + i][j] = ' ';

			}


			if (j == width - 1)
				ScreenData[currentLine + i][j] = '#';
		}
	}

	for ( j = 0; j < width + 2; j++)
	{
		ScreenData[currentLine + i][j] = '#';
	}
	currentLine++;
	sprintf(ScreenData[currentLine + i], "游戏得分：%d", score);
	
}
void Show_DoubleBuffer()
{
	//双缓冲的彩色显示
	HANDLE hBuf;
	WORD textColor;
	int i,j;
	Draw_DoubleBuffer();	//在缓冲区画当前游戏区

	if (BufferSwapFlag == false)
	{
		BufferSwapFlag = true;
		hBuf = hOutBuf;
	}
	else
	{
		BufferSwapFlag = false;
		hBuf = hOutPut;
	}

	//对ScreenData数组的内容进行上色，并将属性传到输出缓冲区hBuf
	for (i = 0; i < height + 5; i++)
	{
		coord.Y = i;
		for (j = 0; j < width + 5; j++)
		{
			coord.X = j;
			if (ScreenData[i][i] == 'O')
			{
				textColor = 0x03;
			}
			else if (ScreenData[i][j] == '@')
			{
				textColor = 0x04;
			}
			else if (ScreenData[i][j] == 'o')
			{
				textColor = 0x0a;
			}
			else
			{
				textColor = 0x06;
			}
			WriteConsoleOutputAttribute(hBuf, &textColor, 1, coord, &bytes);
		}
		coord.X = 0;
		WriteConsoleOutputCharacterA(hBuf, ScreenData[i], width, coord, &bytes);
	}
	SetConsoleActiveScreenBuffer(hBuf);

	//Draw_DoubleBuffer();
	//if (BufferSwapFlag == false)
	//{
	//	BufferSwapFlag = true;
	//	for (int i = 0; i < height + 5; i++)
	//	{
	//		coord.Y = i;
	//		WriteConsoleOutputCharacterA(hOutBuf, ScreenData[i], width, coord, &bytes);
	//	}
	//	SetConsoleActiveScreenBuffer(hOutBuf);
	//}

	//if(BufferSwapFlag == false)
	//{
	//	BufferSwapFlag = true;
	//	for (i = 0; i < height + 5; i++)
	//	{
	//		coord.Y = i;
	//		WriteConsoleOutputCharacterA(hOutBuf, ScreenData[i], width, coord, &bytes);
	//	}
	//	SetConsoleActiveScreenBuffer(hOutBuf);
	//}
	//else
	//{
	//	BufferSwapFlag = false;
	//	for (i = 0; i < height + 5; i++)
	//	{
	//		coord.Y = i;
	//		WriteConsoleOutputCharacterA(hOutPut, ScreenData[i], width, coord, &bytes);
	//	}
	//	SetConsoleActiveScreenBuffer(hOutPut);
	//}
}

//得分提示
void ShowScore(int x, int y) 
{
	setPos(x + STAGE_WIDTH, y + 16);
	SetConsoleTextAttribute(h, COLOR_TEXT);
	cout << "◎当前积分:";
	SetConsoleTextAttribute(h, COLOR_SCORE);
	cout << score;

	setPos(x + STAGE_WIDTH, y + 15);
	SetConsoleTextAttribute(h, COLOR_TEXT);
	cout << "◎当前难度:";
	SetConsoleTextAttribute(h, COLOR_SCORE);
	cout << (int)(score / DIFFICULTY_FACTOR + 1);
}

//提示信息，函数参数接收文字板块在舞台中的原点坐标，通过initialX控制各行文字相对于原点x坐标的缩进
void Prompt_info(int x, int y)
{
	int initialX = STAGE_WIDTH, initialY = 0;

	SetConsoleTextAttribute(h, COLOR_TEXT);
	setPos(x + initialX, y + initialY);
	cout << "■游戏说明：";
	initialY++;
	setPos(x + initialX, y + initialY);
	cout << "    □蛇身自撞，游戏结束";
	initialY++;
	setPos(x + initialX, y + initialY);
	cout << "    □蛇可穿墙";
	initialY++;
	setPos(x + initialX, y + initialY);
	cout << "■操作说明：";
	initialY++;
	setPos(x + initialX, y + initialY);
	cout << "    □向上移动：↑W";
	initialY++;
	setPos(x + initialX, y + initialY);
	cout << "    □向下移动：↓S";
	initialY++;
	setPos(x + initialX, y + initialY);
	cout << "    □向左移动：←A";
	initialY++;
	setPos(x + initialX, y + initialY);
	cout << "    □向右移动：→D";
	initialY++;
	setPos(x + initialX, y + initialY);
	cout << "    □开始游戏：任意操作键";
	initialY++;
	setPos(x + initialX, y + initialY);
	cout << "    □暂停游戏：空格键";
	initialY++;
	setPos(x + initialX, y + initialY);
	cout << "    □退出游戏：x键退出";
	initialY+=10;
	setPos(x + initialX, y + initialY);
	cout << "☆ 作者：杭电数媒 ZQH";
}

//游戏结束界面
void GameOver_info()
{
	//setPos(width/2-4, 8);
	//SetConsoleTextAttribute(h, 0xec);
	//cout << "游戏结束！！";
	//setPos(width/2-6, 9);
	//cout << "Y重新开始/N退出";

	if (isFullWidth)
	{
		setPos(width / 2 - 3, 8);
		SetConsoleTextAttribute(h, COLOR_TEXT_GAMEOVER);
		cout << "游戏结束！！";
		setPos(width / 2 - 4, 9);
		cout << "Y重新开始/N退出";	//输出双字符符号
	}
	else
	{
		setPos(width / 2 - 5, 8);
		SetConsoleTextAttribute(h, COLOR_TEXT_GAMEOVER);
		cout << "游戏结束！！";
		setPos(width / 2 - 7, 9);
		cout << "Y重新开始/N退出";
	}
	SetConsoleTextAttribute(h, COLOR_TEXT);
}

void Draw()
{
	system("cls");
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	int textColor = 0X86;
	SetConsoleTextAttribute(h, textColor);

	for (int i = 0; i < width + 2; i++)//第一行#
	{
		cout << "#";
	}
	cout << endl;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (j == 0)
				cout << "#";

			if (i == fruitY && j == fruitX)
			{
				textColor = 0X81;
				SetConsoleTextAttribute(h, textColor);
				cout << "@";
				textColor = 0X86;
				SetConsoleTextAttribute(h, textColor);
			}
			else if (i == headY && j == headX)
			{
				textColor = 0X8a;
				SetConsoleTextAttribute(h, textColor);
				cout << "O";
				textColor = 0X86;
				SetConsoleTextAttribute(h, textColor);
			}
			else
			{

				bool flagprint = false;
				for (int k = 1; k < ntail; k++)
				{
					if (tailX[k] == j && tailY[k] == i)
					{
						textColor = 0X8a;
						SetConsoleTextAttribute(h, textColor);
						cout << "o";
						flagprint = true;
						break;	//避免蛇身重叠
					}
				}

				textColor = 0X86;
				SetConsoleTextAttribute(h, textColor);
				if (!flagprint) cout << " ";

			}


			if (j == width - 1)
				cout << "#";
		}
		cout << endl;
	}

	for (int i = 0; i < width + 2; i++)
	{
		cout << "#";
	}
	cout << endl;
	cout << "score:" << score << endl;
	cout << ntail;
	//Sleep(200);
}

void Input()
{
	//short a,b;
	//if(a=::GetAsyncKeyState(VK_RETURN))
	//{
	//	dir = UP;
	//}
	//if (b=::GetAsyncKeyState(VK_CONTROL))
	//{
	//	dir = RIGHT;
	//}
	if (_kbhit())
	{
		//spaceReadyFlag的设置是为了第二次按下空格的时候（暂停时按空格）也能继续游戏
		bool spaceReadyFlag = true;
		if (isPause == true) {
			spaceReadyFlag = false;
		}
		isPause = false;

		switch (_getch())
		{
		case' ':
			if (spaceReadyFlag == true) {
				isPause = true;
			}
			break;
		case'w':
			if (dir != DOWN)
				dir = UP;
			break;
		case's':
			if (dir != UP)
				dir = DOWN;
			break;
		case'a':
			if (dir != RIGHT)
				dir = LEFT;
			break;
		case'd':
			if (dir != LEFT)
				dir = RIGHT;
			break;
		case'x':
			gameOver = true;
			break;
		case'W':
			if (dir != DOWN)
				dir = UP;
			break;
		case'S':
			if (dir != UP)
				dir = DOWN;
			break;
		case'A':
			if (dir != RIGHT)
				dir = LEFT;
			break;
		case'D':
			if (dir != LEFT)
				dir = RIGHT;
			break;
		case'X':
			gameOver = true;
			break;

		case 224:	//方向键的ASCII码
			switch (_getch())
			{
			case 72:
				if (dir != DOWN)
					dir = UP;
				break;
			case 80:
				if (dir != UP)
					dir = DOWN;
				break;
			case 75:
				if (dir != RIGHT)
					dir = LEFT;
				break;
			case 77:
				if (dir != LEFT)
					dir = RIGHT;
				break;
			default:
				break;
			}
			break;

		case 0x0D:	//回车键
			if (isFullWidth)
			{
				isFullWidth = false;
			}
			else
			{
				isFullWidth = true;
			}
			updateMap = true;
			/*DrawMap();
			Prompt_info(5, 1);*/
			break;
		case'j':
		case'J':
			isPause = true;
			break;
		case'k':
		case'K':
			isPause = false;
			break;
		default:
			break;
		}
	}
}

void Logic()
{
	tailX[0] = headX;
	tailY[0] = headY;

	//update snack head
	switch (dir)
	{
	case UP:
		headY--;
		break;
	case DOWN:
		headY++;
		break;
	case LEFT:
		headX--;
		break;
	case RIGHT:
		headX++;
		break;
	default:
		break;
	}

	//穿墙
	if (headX >= width)headX = 0;
	else if (headX < 0)headX = STAGE_WIDTH - 1;
	if (headY >= height)headY = 0;
	else if (headY < 0)headY = STAGE_HEIGHT - 1;

	//撞墙则gameover
	/*if (headX > STAGE_WIDTH - 1 || headX < 0 || headY > STAGE_HEIGHT - 1 || headY < 0)
	{
		gameOver = true;
	}*/
	//蛇头与蛇身相撞则gameover
	for (int i = 1; i < ntail; i++)
	{
		if (tailX[i] == headX && tailY[i] == headY)
			gameOver = true;
	}

	//get fruit
	if (headX == fruitX && headY == fruitY)
	{
		score += 10;
		int originX = fruitX;
		int originY = fruitY;
		bool flag = false;
		while (!flag)
		{
			fruitX = rand() % STAGE_WIDTH;
			fruitY = rand() % STAGE_HEIGHT;
			flag = true;
			for (int i = 1; i < ntail; i++)
			{
				if ((fruitX == tailX[i] && fruitY == tailY[i]) || (fruitX == originX && fruitY == originY))	//新生成的fruit在蛇身上
				{
					flag = false;
					break;
				}
			}
		}
		ntail++;
	}


	//update snack body
	int prevX = tailX[0];
	int prevY = tailY[0];
	int prev2X, prev2Y;

	//将蛇身位置挨个往后传（去除掉了最后一个位置
	for (int i = 1; i < ntail; i++)
	{
		prev2X = tailX[i];
		prev2Y = tailY[i];
		tailX[i] = prevX;
		tailY[i] = prevY;
		prevX = prev2X;
		prevY = prev2Y;
	}

	

}

//-------------------局部更新

//游戏场景的绘制
void DrawMap()
{
	system("cls");
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	int textColor = COLOR_WALL;
	SetConsoleTextAttribute(h, textColor);

	//绘制顶上的墙
	setPos(-THICHKNESS, -THICHKNESS);
	for (int i = 0; i < STAGE_WIDTH + THICHKNESS * 2; i++)
	{
		if (isFullWidth)
		{
			cout << "□";	//输出双字符符号
		}
		else
		{
			cout << "#";
		}
	}

	//绘制左右的墙
	for (int i = 0; i < STAGE_HEIGHT; i++)
	{
		setPos(-1, i);
		for (int j = 0; j < STAGE_WIDTH + THICHKNESS * 2; j++)
		{
			if (j == 0)
			{
				if (isFullWidth)
				{
					cout << "□";	//输出双字符符号
				}
				else
				{
					cout << "#";
				}
			}
			else if (j == STAGE_WIDTH + THICHKNESS * 1)
			{
				if (isFullWidth)
				{
					cout << "□";	//输出双字符符号
				}
				else
				{
					cout << "#";
				}
			}
			else
			{
				if (isFullWidth)
				{
					cout << "  ";	//输出双字符符号
				}
				else
				{
					cout << " ";
				}
			}
		}
	}

	//绘制下方的墙
	setPos(-THICHKNESS, STAGE_HEIGHT);
	for (int i = 0; i < STAGE_WIDTH + THICHKNESS * 2; i++)
	{
		if (isFullWidth)
		{
			cout << "□";	//输出双字符符号
		}
		else
		{
			cout << "#";
		}
	}
	cout << endl;
	//cout << "游戏得分" << score << endl;
}

//局部擦除
void EarseSnake()
{
	setPos(headX, headY);
	//cout << " ";
	if (isFullWidth)
	{
		cout << "  ";	//输出双字符符号
	}
	else
	{
		cout << " ";
	}
	for (int i = 1; i < ntail; i++)
	{
		setPos(tailX[i], tailY[i]);
		//cout << " ";
		if (isFullWidth)
		{
			cout << "  ";	//输出双字符符号
		}
		else
		{
			cout << " ";
		}
	}
}

//局部绘制
void DrawLocally()
{
	if (updateMap) {
		updateMap = false;
		DrawMap();
		Prompt_info(5, 1);
	}
	if (!fruitFlash)
	{
		setPos(fruitX, fruitY);
		SetConsoleTextAttribute(h, COLOR_FRUIT);
		if (isFullWidth)
		{
			cout << "★";	//输出双字符符号
		}
		else
		{
			cout << "@";
		}
		fruitFlash = true;
	}
	else
	{
		setPos(fruitX, fruitY);
		SetConsoleTextAttribute(h, COLOR_FRUIT);
		if (isFullWidth)
		{
			cout << "  ";	//输出双字符符号
		}
		else
		{
			cout << " ";
		}
		fruitFlash = false;
	}

	SetConsoleTextAttribute(h, COLOR_SNAKE_HEAD);
	setPos(headX, headY);
	//cout << "O";
	if (isFullWidth)
	{
		cout << "●";	//输出双字符符号
	}
	else
	{
		cout << "O";
	}
	for (int i = 1; i < ntail; i++)
	{
		setPos(tailX[i], tailY[i]);
		SetConsoleTextAttribute(h, COLOR_SNAKE_BODY);
		if (isFullWidth)
		{
			cout << "○";	//输出双字符符号
		}
		else
		{
			cout << "o";
		}
		/*	if (i == 0)
			{
				SetConsoleTextAttribute(h, 0x09);
				cout << "O";
			}
			else
			{
				SetConsoleTextAttribute(h, 0x0a);
				cout << "o";
			}*/
	}

	setPos(0, STAGE_HEIGHT + THICHKNESS * 2);
	SetConsoleTextAttribute(h, COLOR_TEXT);
	cout << "游戏得分:" << score << endl;
}

int main()
{
	
	bool gameQuit = false;
	do {
		Initial();
		DrawMap();	//游戏场景的绘制
		Prompt_info(3, 1);	//提示信息,x参考系为游戏右边框

		while (!gameOver)
		{
			//Draw();
			//Show_DoubleBuffer();		
			Input();
			
			EarseSnake();	//局部擦除
			if (!isPause)
			{
				Logic();
			}
			DrawLocally();	//局部绘制

			ShowScore(3, 1);	//显示得分

			//恒定帧率
			/*next_Game_Tick += SKIP_TICKS;
			sleep_Time = next_Game_Tick - GetTickCount();
			if (sleep_Time >= 0)
				Sleep(sleep_Time);*/

			//动态帧率
			sleep_Time = 200 / (score / DIFFICULTY_FACTOR + 1);
			Sleep(sleep_Time);

			//Sleep(100);
			//tail[0]!=head
			/*for (int i = 0; i < 7; i++)
			{
				cout << endl;
			}
			cout << "head:";
			cout << headX <<";"<< headY << endl;
			cout << "tail[0]:";
			cout << tailX[0] << ";" << tailY[0] << endl;*/
		}
		GameOver_info();

		while (gameOver)
		{
			if (_kbhit())
			{
				switch (_getch())
				{
				case'y':
				case'Y':
					gameOver = false;
					system("cls");
					break;
				case'n':
				case'N':
					gameOver = false;
					gameQuit = true;
					break;
				default:
					break;
				}
			}
			Sleep(50);
		}
	} while (!gameQuit);
	

	setPos(0, STAGE_HEIGHT + MAXLENGTH * 3);

	system("pause");

	return 0;
}