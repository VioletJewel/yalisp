
#ifndef VECTOR_INIT_CAPACITY
#define VECTOR_INIT_CAPACITY 16
#endif

#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

struct vector {
    void *items;
    size_t itemsize;
    size_t capacity;
    size_t size;
};

void vector_init(struct vector *, size_t);
size_t vector_size(struct vector *);

void* vector_insert(struct vector *, size_t, void *);
void vector_remove(struct vector *, size_t, void *);

void* vector_set(struct vector *, size_t, void *);
void* vector_get(struct vector *, size_t);

void* vector_push(struct vector *, void *);
void vector_pop(struct vector *, void *);

void vector_clear(struct vector *);
void vector_free(struct vector *);

#endif

