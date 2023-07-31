#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <stdint.h>
#include <Windows.h>

typedef struct timer
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER prev_counter;

    float interval;
    float elapsed_tick;
} timer_t;

bool init_timer(timer_t* p_timer, const float interval);

void update_timer(timer_t* p_timer);

void reset_timer(timer_t* p_timer);

bool is_on_tick_timer(const timer_t* p_timer);

#endif // TIMER_H