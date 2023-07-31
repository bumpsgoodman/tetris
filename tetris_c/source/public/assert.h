#ifndef ASSERT_H
#define ASSERT_H

#if defined(NDBUG)
#define ASSERT(cond, msg) ((void)0)
#else
#include <intrin.h>
#define ASSERT(cond, msg) { if (!(cond)) { __debugbreak(); } }
#endif // NDBUG

#endif // ASSERT_H