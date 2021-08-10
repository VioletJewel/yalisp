
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "vector.h"
#include "node.h"

void
test_vector()
{
    size_t ind, size;
    void *addr;
    long long int lli;
    struct vector v;

    vector_init(&v, sizeof(long long int));

    for (lli = 0; lli < 20; ++lli)
        vector_push(&v, &lli);

    for (ind = 0, size = vector_size(&v); ind < size; ++ind) {
        addr = vector_get(&v, ind);
        lli = *(long long int*)addr;
        assert(lli == ind);
    }

    vector_remove(&v, 4, &lli);
    assert(lli == 4);
    vector_remove(&v, 1, &lli);
    assert(lli == 1);
    vector_remove(&v, 2, &lli);
    assert(lli == 3);

    vector_remove(&v, 0, NULL);
    vector_remove(&v, 0, NULL);

    vector_pop(&v, &lli);
    assert(lli == vector_size(&v) + 5);
    vector_pop(&v, &lli);
    assert(lli == vector_size(&v) + 5);

    for (ind = 0, size = vector_size(&v); ind < size; ++ind) {
        addr = vector_get(&v, ind);
        lli = *(long long int*)addr;
        lli -= 5;
        addr = vector_set(&v, ind, &lli);
        lli = *(long long int*)addr;
        addr = vector_get(&v, ind);
        lli = *(long long int*)addr;
        assert(lli == ind);
        lli += 5;
        addr = vector_set(&v, ind, &lli);
        lli = *(long long int*)addr;
    }

    for (lli = 0; lli < 5; ++lli)
        vector_insert(&v, lli, &lli);

    for (ind = 0, size = vector_size(&v); ind < size; ++ind) {
        vector_remove(&v, 0, &lli);
        assert(lli == ind);
    }

    vector_free(&v);
}

void
test_node()
{
    struct node node, child, sibling;

    node_init(&node);
    assert(node_get_type(&node) == T_UNDEFINED);
    assert(node_get_sibling(&node) == NULL);
    assert(node_get_child(&node) == NULL);

    node_init(&child);
    assert(node_get_type(&child) == T_UNDEFINED);
    assert(node_get_sibling(&child) == NULL);
    assert(node_get_child(&child) == NULL);

    node_init(&sibling);
    assert(node_get_type(&sibling) == T_UNDEFINED);
    assert(node_get_sibling(&sibling) == NULL);
    assert(node_get_child(&sibling) == NULL);

    node_set(&node, T_INT, (union node_data){ .i = -42 });
    assert(node_get_type(&node) == T_INT);
    assert(node_get_data(&node).i == -42);

    node_set(&child, T_CHAR, (union node_data){ .c = 'c' });
    assert(node_get_type(&child) == T_CHAR);
    assert(node_get_data(&child).c == 'c');

    node_set(&sibling, T_CHAR, (union node_data){ .c = 's' });
    assert(node_get_type(&sibling) == T_CHAR);
    assert(node_get_data(&sibling).c == 's');

    node_set_child(&node, &child);
    assert(node_get_child(&node) == &child);
    assert(node_get_type(node_get_child(&node)) == T_CHAR);
    assert(node_get_data(node_get_child(&node)).c == 'c');

    node_set_sibling(&node, &sibling);
    assert(node_get_sibling(&node) == &sibling);
    assert(node_get_type(node_get_sibling(&node)) == T_CHAR);
    assert(node_get_data(node_get_sibling(&node)).c == 's');
}

int
main(void)
{
    test_vector();
    test_node();
    puts("all tests passed :)");
}
