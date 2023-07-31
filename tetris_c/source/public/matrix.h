#ifndef MATRIX_H
#define MATRIX_H

#include <stdint.h>

typedef struct int_vec2
{
    int32_t x;
    int32_t y;
} int_vec2_t;

typedef struct int_mat2
{
    int_vec2_t row0;
    int_vec2_t row1;
} int_mat2_t;

void transpose22(int_mat2_t* p_mat);

void transform22(int_vec2_t* p_dst, const int_vec2_t* p_src, const int_mat2_t* p_mat_tr);

#endif // MATRIX_H