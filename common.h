#ifndef __COMMON_H__
#define __COMMON_H__

#define EQ(S, T) (strcmp((S), (T)) == 0)
#define DEFAULT_ADDRESS "localhost"

#define ALLOCATE(TYPE, VAR) \
    TYPE *VAR = (TYPE*) calloc(1, sizeof(TYPE)); \
    assert(VAR != NULL)

#endif //__COMMON_H__

