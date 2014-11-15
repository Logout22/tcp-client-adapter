#ifndef __COMMON_H__
#define __COMMON_H__

#include "config.h"

#define EQ(S, T) (strcmp((S), (T)) == 0)

#define ALLOCATE(TYPE, VAR) \
    TYPE *VAR = (TYPE*) calloc(1, sizeof(TYPE)); \
    assert(VAR != NULL)

#endif

