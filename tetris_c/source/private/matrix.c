#include "../public/matrix.h"
#include "../public/assert.h"

void transpose22(int_mat2_t* p_mat)
{
    ASSERT(p_mat != NULL, "p_mat == NULL");

    const int32_t temp = p_mat->row0.y;
    p_mat->row0.y = p_mat->row1.x;
    p_mat->row1.x = temp;
}

void transform22(int_vec2_t* p_dst, const int_vec2_t* p_src, const int_mat2_t* p_mat_tr)
{
    ASSERT(p_src != NULL, "p_src == NULL");
    ASSERT(p_dst != NULL, "p_dst == NULL");
    ASSERT(p_mat_tr != NULL, "p_mat_tr == NULL");

    p_dst->x = p_src->x * p_mat_tr->row0.x + p_src->y * p_mat_tr->row0.y;
    p_dst->y = p_src->x * p_mat_tr->row1.x + p_src->y * p_mat_tr->row1.y;
}