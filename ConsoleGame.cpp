#define _WIN32_WINNT 0x0500
#include <Windows.h>
#include <conio.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>

#define MAX_BULLETS 5
#define MAX_ASTR 6

HANDLE hConsole;
CONSOLE_CURSOR_INFO cursorInfo;
CONSOLE_SCREEN_BUFFER_INFO screenInfo;
HANDLE hPThread,inputThread,collisionT,generatorT;
bool ammo[MAX_BULLETS];
bool astr[MAX_ASTR];
int key,score=0;
SHORT x1 = 20, y1 = 48;
WORD blk = 0x0F;
bool close = false,paus = false;


typedef struct asteroid
{
	SHORT x, y;
	DWORD speed;
	HANDLE Thread;
	bool dest;
}ASTR;

typedef struct bullet
{
	SHORT x, y;
	HANDLE Thread;
	bool dest;

}BULLET;

BULLET B[MAX_BULLETS];
ASTR A[MAX_ASTR];

void cls()
{
	DWORD dummy;
	FillConsoleOutputCharacter(hConsole, ' ', screenInfo.dwSize.X * screenInfo.dwSize.Y, { 0,0 }, &dummy);
	FillConsoleOutputAttribute(hConsole, 0x0F, screenInfo.dwSize.X*screenInfo.dwSize.Y, { 0,0 }, &dummy);
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void closeHandles()
{
	CloseHandle(hPThread);
	for(int i=0;i<MAX_BULLETS;i++)
	    CloseHandle(B[i].Thread);
	for (int i = 0; i < MAX_ASTR; i++)
		CloseHandle(A[i].Thread);
	CloseHandle(inputThread);
	CloseHandle(collisionT);
	CloseHandle(generatorT);
}

void suspendALL()
{
    SuspendThread(generatorT);
	SuspendThread(hPThread);
	for(int i=0;i<MAX_BULLETS;i++)
	SuspendThread(B[i].Thread);
	for (int i = 0; i<MAX_ASTR; i++)
	SuspendThread(A[i].Thread);
	SuspendThread(collisionT);
	SuspendThread(inputThread);
}

void pause()
{
	paus = true;
	SuspendThread(generatorT);
	SuspendThread(hPThread);
	for(int i=0;i<MAX_BULLETS;i++)
	SuspendThread(B[i].Thread);
	for (int i = 0; i<MAX_ASTR; i++)
	SuspendThread(A[i].Thread);
	SuspendThread(collisionT);
	cls();
	ResumeThread(inputThread);
}


void unpause()
{
	paus = false;
	ResumeThread(hPThread);
	for (int i = 0; i<MAX_BULLETS; i++)
		ResumeThread(B[i].Thread);
	for (int i = 0; i<MAX_ASTR; i++)
		ResumeThread(A[i].Thread);
	ResumeThread(collisionT);
	ResumeThread(generatorT);
	cls();
	SuspendThread(inputThread);

}


HANDLE WINAPI threadMov(LPVOID lpParam)
{
	DWORD dummy;
	WORD color = 0x05;
	while (1)
	{

		if (key == 0x6B)
		{
			if (x1 > 1)
			{
				x1--;
				WriteConsoleOutputCharacter(hConsole, " ", 1, { x1+1,y1 }, &dummy);
				WriteConsoleOutputCharacter(hConsole, "  ", 2, { x1+1 ,y1 + 1 }, &dummy);
				WriteConsoleOutputAttribute(hConsole, &blk, 1, { x1+1,y1 + 1 }, &dummy);

			}
		}
		if (key == 0x6D)
		{
			if (x1 < 55)
			{
				x1++;
				WriteConsoleOutputCharacter(hConsole, " ", 1, { x1-1,y1 }, &dummy);
				WriteConsoleOutputCharacter(hConsole, "  ", 2, { x1-2 ,y1 + 1 }, &dummy);
				WriteConsoleOutputAttribute(hConsole, &blk, 1, { x1-1,y1 + 1 }, &dummy);

			}
		}
		WriteConsoleOutputCharacter(hConsole, "^", 1, { x1,y1 }, &dummy);
		WriteConsoleOutputCharacter(hConsole, "^+^", 3, { x1 - 1,y1 + 1 }, &dummy);
		WriteConsoleOutputAttribute(hConsole, &color, 1, { x1,y1 + 1 }, &dummy);
		Sleep(50);
		color++;
		if (color > 0x06) color = 0x04;

	}
	return 0;
}

HANDLE WINAPI threadAstr(LPVOID lpParam)
{
	DWORD dummy;
	int ind = (int)lpParam;
	A[ind].dest=false;
	while (1)
	{
		WriteConsoleOutputCharacter(hConsole, "O", 1, { A[ind].x,A[ind].y }, &dummy);
		Sleep(A[ind].speed);
		A[ind].y++;
		WriteConsoleOutputCharacter(hConsole, " ", 1, { A[ind].x,A[ind].y-1 }, &dummy);
		if (A[ind].y >= 50)
		{
			astr[ind]=0;
			close=true;
			ExitThread(dummy);
		}
		if(A[ind].dest)
            {
              score++;
              astr[ind]=0;
              ExitThread(dummy);
            }
	}
	return 0;
}

HANDLE WINAPI generator(LPVOID lpParam)
{
	DWORD dwGen;
	srand(time(NULL));
	while (1)
	{
		Sleep(2000);
		if (!paus)
		{
			for (int i = 0; i < MAX_ASTR; i++) if (!astr[i]) { A[i].x = (rand() % 40) + 1; A[i].y = 0; A[i].speed = (rand() % 1000) + 100; A[i].Thread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(threadAstr), (void*)i, 0, &dwGen); astr[i] = 1; i = MAX_ASTR; }
		}
	}
	return 0;
}

HANDLE WINAPI threadBullet(LPVOID lpParam)
{
	DWORD dummy;
	int ind = (int)lpParam;
	WORD color = 0x0C;
	B[ind].x = x1; B[ind].y = y1 - 1; B[ind].dest=false;
	while (1)
	{
		WriteConsoleOutputCharacter(hConsole, "|", 1, { B[ind].x,B[ind].y }, &dummy);
		WriteConsoleOutputAttribute(hConsole, &color, 1, { B[ind].x,B[ind].y }, &dummy);
		Sleep(50);
		B[ind].y--;
		WriteConsoleOutputCharacter(hConsole, " ", 1, { B[ind].x,B[ind].y + 1 }, &dummy);
		WriteConsoleOutputAttribute(hConsole, &blk, 1, { B[ind].x,B[ind].y + 1 }, &dummy);
		if (B[ind].y <= 0)
		{
			ammo[ind]=0;
			ExitThread(dummy);

		}
		if(B[ind].dest)
          {
            ammo[ind]=0;
            WriteConsoleOutputCharacter(hConsole," ",1,{B[ind].x,B[ind].y},&dummy);
            WriteConsoleOutputAttribute(hConsole,&blk,1,{B[ind].x,B[ind].y},&dummy);
            B[ind].x=-5;
            ExitThread(dummy);
          }

	}
	return 0;
}
HANDLE WINAPI input(LPVOID lpParam)
{
	DWORD dummy;
	WORD color1 =0x1F,color2 =0x0F;
	while (1)
	{
		if (key == 0x68) { color1 = 0x1F; color2 = 0x0F; }
		if (key == 0x70) { color1 = 0x0F; color2 = 0x1F; }
		if (key == 0x0D)
		{
		   if(color1==0x1F) unpause();
		   else close = true;
		}
		WriteConsoleOutputCharacter(hConsole, "P A U S E D", 11, { 22,9 }, &dummy);
		WriteConsoleOutputCharacter(hConsole, "##########", 10, { 22,15 }, &dummy);
		WriteConsoleOutputCharacter(hConsole, "# RESUME #", 10, { 22,16 }, &dummy);
		WriteConsoleOutputCharacter(hConsole, "##########", 10, { 22,17 }, &dummy);
		WriteConsoleOutputCharacter(hConsole, "##########", 10, { 22,25 }, &dummy);
		WriteConsoleOutputCharacter(hConsole, "#  EXIT  #", 10, { 22,26 }, &dummy);
		WriteConsoleOutputCharacter(hConsole, "##########", 10, { 22,27 }, &dummy);
		for (SHORT i = 22; i < 32; i++)
		{
			WriteConsoleOutputAttribute(hConsole, &color1, 1, { i,15 }, &dummy);
			WriteConsoleOutputAttribute(hConsole, &color1, 1, { i,16 }, &dummy);
			WriteConsoleOutputAttribute(hConsole, &color1, 1, { i,17 }, &dummy);
			WriteConsoleOutputAttribute(hConsole, &color2, 1, { i,25 }, &dummy);
			WriteConsoleOutputAttribute(hConsole, &color2, 1, { i,26 }, &dummy);
			WriteConsoleOutputAttribute(hConsole, &color2, 1, { i,27 }, &dummy);
		}
		Sleep(50);
	}
}

HANDLE WINAPI collision(LPVOID lpParam)
{
	DWORD dummy;
	char output[10]="";
    sprintf(output, "SCORE: %d", score);
	while (1)
	{
		for(int i=0;i<MAX_BULLETS;i++)
			for(int j=0;j<MAX_ASTR;j++)
				if (B[i].y == A[j].y && B[i].x == A[j].x)
				{

					B[i].dest=true;
                    A[j].dest=true;

				}
    sprintf(output, "SCORE: %d", score);
    WriteConsoleOutputCharacter(hConsole, output, 10, { 46,2 }, &dummy);
    Sleep(50);

	}
	return 0;
}

int main(int argc, char* argv[])
{
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTitle("Console Game");
	DWORD dummy;
	HWND hwnd = GetConsoleWindow();
	RECT rect = { 100, 100, 800, 600 };
	MoveWindow(hwnd, rect.top, rect.left, rect.bottom - rect.top, rect.right - rect.left, TRUE);
	GetConsoleScreenBufferInfo(hConsole, &screenInfo);
	SetConsoleScreenBufferSize(hConsole, {0,0});
	DWORD dwPThread,dwBThread,dwInput,dwAThread,dwCol,dwGen;
	for (int i = 0; i < MAX_BULLETS; i++)
    {
       ammo[i] = 0;
       B[i].y = -5;
    }
	inputThread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(input), NULL, 0, &dwInput);
	SuspendThread(inputThread);
	hPThread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(threadMov), NULL, 0, &dwPThread);
	collisionT = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(collision), NULL, 0, &dwCol);
	generatorT = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(generator), NULL, 0, &dwGen);
	cls();
	while(!close)
	{

		key = tolower(_getch());
		if (key == 0x1B) {if(!paus) pause(); }
		if (key == 0x20)
		{
			if (!paus) { for (int i = 0; i < MAX_BULLETS; i++) if (!ammo[i]) { B[i].Thread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(threadBullet), (void*)i, 0, &dwBThread); ammo[i] = 1; i = MAX_BULLETS; } }
		}

	}

	if(!paus)
    {
	suspendALL();
	cls();
	WriteConsoleOutputCharacter(hConsole, "FINAL SCORE", 12, { 22,22 }, &dummy);
	char finalScore[10]="      ";
	sprintf(finalScore,"%d",score);
    WriteConsoleOutputCharacter(hConsole,finalScore, 5, { 28,25 }, &dummy);
    Sleep(5000);
    }
    closeHandles();
}
