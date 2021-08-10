

#include <stdlib.h>
#include <string.h>

#include "vector.h"

static void vector_resize(struct vector *vector, size_t capacity);

void
vector_init(struct vector *vector, size_t itemsize)
{
    vector->capacity = VECTOR_INIT_CAPACITY;
    vector->itemsize = itemsize;
    vector->size = 0;
    vector->items = malloc(VECTOR_INIT_CAPACITY * itemsize);
    if (!vector->items)
        vector->capacity = 0;
}

size_t
vector_size(struct vector *vector)
{
    return vector->size;
}

static void
vector_resize(struct vector *vector, size_t capacity)
{
    void *items;
    items = realloc(vector->items, capacity * vector->itemsize);
    if (items) {
        #ifdef DEBUG_ON
        fprintf(stderr, "vector resized from %zu to %zu\n", vector->capacity, capacity);
        #endif
        vector->items = items;
        vector->capacity = capacity;
    } else {
        #ifdef DEBUG_ON
        fprintf(stderr, "failed to resize vector from %zu to %zu\n", vector->capacity, capacity);
        #endif
    }
}

void *
vector_insert(struct vector *vector, size_t index, void *item) {
    size_t ind;
    void *addr;
    if (index > vector->size || !vector->capacity)
        return NULL;
    if (vector->size == vector->capacity) {
        vector_resize(vector, vector->capacity * 2);
        if (!vector->capacity)
            return NULL;
    }
    for (ind = vector->size; ind > index; --ind) {
        addr = vector->items + ind * vector->itemsize;
        memcpy(addr, addr - vector->itemsize, vector->itemsize);
    }
    addr = vector->items + index * vector->itemsize;
    memcpy(addr, item, vector->itemsize);
    ++vector->size;
    return addr;
}

void
vector_remove(struct vector *vector, size_t index, void *item)
{
    size_t ind;
    void *addr;
    if (index >= vector->size || !vector->capacity)
        return;
    if (item) /* if item is NULL, don't copy into */
        memcpy(item, vector_get(vector, index), vector->itemsize);
    if (vector->capacity > VECTOR_INIT_CAPACITY && vector->size * 4 <= vector->capacity) {
        vector_resize(vector, vector->capacity / 4);
        if (!vector->capacity)
            return;
    }
    for (ind = index; ind < vector->size; ++ind) {
        addr = vector->items + ind * vector->itemsize;
        memcpy(addr, addr + vector->itemsize, vector->itemsize);
    }
    addr = vector->items + ind * vector->itemsize;
    memset(addr, 0, vector->itemsize);
    --vector->size;
}

void *
vector_set(struct vector *vector, size_t index, void *item)
{
    void *addr = NULL;
    if (vector->capacity && index < vector->size) {
        addr = vector->items + index * vector->itemsize;
        memcpy(addr, item, vector->itemsize);
    }
    return addr;
}

void *
vector_get(struct vector *vector, size_t index)
{
    if (vector->capacity && index < vector->size)
        return vector->items + index * vector->itemsize;
    return NULL;
}

void *
vector_push(struct vector *vector, void *item)
{
    return vector_insert(vector, vector->size, item);
}

void
vector_pop(struct vector *vector, void *item)
{
    return vector_remove(vector, vector->size - 1, item);
}

void
vector_clear(struct vector *vector)
{
    vector->size = 0;
    /*
    if (vector->size != VECTOR_INIT_CAPACITY)
        vector_resize(vector, VECTOR_INIT_CAPACITY);
    */
}

void
vector_free(struct vector *vector)
{
    free(vector->items);
}

