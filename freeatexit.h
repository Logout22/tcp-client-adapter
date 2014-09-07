#ifndef __FREEATEXIT_H__
#define __FREEATEXIT_H__

#include "common.h"

typedef struct freeable_object {
    void (*ptr_free_fn)(void*);
    void *object;
    struct freeable_object *next;
} freeable_object;

void free_atexit(void);
void free_object_at_exit(void (*ptr_free_fn)(void*), void *object);

#endif

