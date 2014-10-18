#include "freeatexit.h"
#include <stdlib.h>
#include <assert.h>

static freeable_object *objects_to_free = NULL;

void free_object_at_exit(void (*ptr_free_fn)(void*), void *object) {
    ALLOCATE(freeable_object, new_obj);

    new_obj->ptr_free_fn = ptr_free_fn;
    new_obj->object = object;

    /* prepend to free objects in reverse order of allocation */
    new_obj->next = objects_to_free;
    objects_to_free = new_obj;
}

void free_atexit() {
    while (objects_to_free != NULL) {
        objects_to_free->ptr_free_fn(objects_to_free->object);
        freeable_object *next = objects_to_free->next;
        free(objects_to_free);
        objects_to_free = next;
    }
}

