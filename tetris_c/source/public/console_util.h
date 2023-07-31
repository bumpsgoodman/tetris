#ifndef CONSOLE_UTIL_H
#define CONSOLE_UTIL_H

#include <stdbool.h>
#include <stdint.h>
#include <Windows.h>

void gotoxy(const uint32_t x, const uint32_t y);

void set_visible_cursor(const bool b_visible);

#endif // CONSOLE_UTIL_H