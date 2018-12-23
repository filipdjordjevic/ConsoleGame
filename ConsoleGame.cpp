#include <Windows.h>
#include <conio.h>

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
CONSOLE_CURSOR_INFO cursorInfo;
CONSOLE_SCREEN_BUFFER_INFO screenInfo;
int key;
SHORT x1=20,x2=19,y1=20,y2=21,x3,y3; 
SHORT meniY=15;
HANDLE hPThread, hBThread, inputThread;
bool close = false;


void cls()
{
	DWORD dummy;
	COORD Home = { 0,0 };
	FillConsoleOutputCharacter(hConsole, ' ', screenInfo.dwSize.X * screenInfo.dwSize.Y, Home, &dummy);
	FillConsoleOutputAttribute(hConsole, 0x0F, screenInfo.dwSize.X*screenInfo.dwSize.Y, Home, &dummy);
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void pause()
{
	SuspendThread(hPThread);
	SuspendThread(hBThread);
	ResumeThread(inputThread);
}

void unpause()
{
	
	ResumeThread(hPThread);
	ResumeThread(hBThread);
	SuspendThread(inputThread);
}

HANDLE WINAPI threadMov(LPVOID lpParam)
{
	DWORD dummy;
	WORD color = 0x05;
	while (1)
	{
		
		cls();
		WriteConsoleOutputCharacter(hConsole, L"^", 1, { x1,y1 }, &dummy);
		WriteConsoleOutputCharacter(hConsole, L"^+^", 3, { x2,y2 }, &dummy);
		WriteConsoleOutputAttribute(hConsole, &color, 1, {x2+1,y2},&dummy);
		Sleep(25);

		color++;
		if (color > 0x06) color = 0x04;
		
	}
	return 0;
}

HANDLE WINAPI threadBullet(LPVOID lpParam)
{
	DWORD dummy;
	WORD color = 0x0C;
	while (1)
	{
		WriteConsoleOutputCharacter(hConsole, L"|", 1, { x3,y3 }, &dummy);
		WriteConsoleOutputAttribute(hConsole, &color, 1, { x3,y3 }, &dummy);
		y3--;
		Sleep(25);
	}
	return 0;
}
HANDLE WINAPI input(LPVOID lpParam)
{
	DWORD dummy;
	WORD color =0x1F;

	while (1)
	{
		cls();
		if (key == 0x68) meniY=15;
		if (key == 0x70) meniY=25;
		if (key == 0x0D)
		{
		   if(meniY==15) unpause();
		   else close = true;
		}
		WriteConsoleOutputCharacter(hConsole, L"P A U S E D", 11, { 22,9 }, &dummy);
		WriteConsoleOutputCharacter(hConsole, L"##########", 10, { 22,15 }, &dummy);
		WriteConsoleOutputCharacter(hConsole, L"# RESUME #", 10, { 22,16 }, &dummy);
		WriteConsoleOutputCharacter(hConsole, L"##########", 10, { 22,17 }, &dummy);
		WriteConsoleOutputCharacter(hConsole, L"##########", 10, { 22,25 }, &dummy);
		WriteConsoleOutputCharacter(hConsole, L"#  EXIT  #", 10, { 22,26 }, &dummy);
		WriteConsoleOutputCharacter(hConsole, L"##########", 10, { 22,27 }, &dummy);
		for (SHORT i = 22; i < 32; i++)
		{
			WriteConsoleOutputAttribute(hConsole, &color, 1, { i,meniY }, &dummy);
			WriteConsoleOutputAttribute(hConsole, &color, 1, { i,meniY + 1 }, &dummy);
			WriteConsoleOutputAttribute(hConsole, &color, 1, { i,meniY + 2 }, &dummy);
		}
		Sleep(50);
	}
}


int main(int argc, char* argv[])
{
	SetConsoleTitle(L"Console Game");
	HWND hwnd = GetConsoleWindow();
	RECT rect = { 100, 100, 800, 600 };
	MoveWindow(hwnd, rect.top, rect.left, rect.bottom - rect.top, rect.right - rect.left, TRUE);
	GetConsoleScreenBufferInfo(hConsole, &screenInfo);
	SetConsoleScreenBufferSize(hConsole, {0,0});
	DWORD dwPThread,dwBThread,dwInput;
	hPThread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(threadMov), NULL, 0, &dwPThread);
	hBThread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(threadBullet), NULL, 0, &dwBThread);
    inputThread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(input), NULL, 0, &dwInput);
	SuspendThread(inputThread);
	while(!close)
	{
		key = tolower(_getch());
		if (key == 0x6B) { x1--; x2--; }
		if (key == 0x6D) { x1++; x2++; }
		if (key == 0x68) { y1--; y2--; }
		if (key == 0x70) { y1++; y2++; }
		if (key == 0x1B) { pause(); }
		if (key == 0x20) { y3 = y1 - 1; x3 = x1; }
		
	}
	CloseHandle(hPThread);
	CloseHandle(hBThread);
	CloseHandle(inputThread);
}
