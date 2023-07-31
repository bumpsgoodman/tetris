#include "../public/console_util.h"

void gotoxy(const uint32_t x, const uint32_t y)
{ 
    const COORD pos={ x, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos); 
}

void set_visible_cursor(const bool b_visible)
{
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = b_visible;
    SetConsoleCursorInfo(consoleHandle, &info);
}