
#ifndef CLISP_TREE_H
#define CLISP_TREE_H

#include <stdlib.h>

#define NODE_INIT(node)                    node_init(&node)
#define NODE_GET_TYPE(node)                node_get_type(&node)
#define NODE_GET_DATA(node)                node_get_data(&node)
#define NODE_GET_SIBLING(node)             node_get_sibling(&node)
#define NODE_GET_CHILD(node)               node_get_child(&node)
#define NODE_SET(node, type, stype, data)  node_set(&node, type, (union node_data){ .stype = data })
#define NODE_SET_SIBLING(node, sib)        node_set_sibling(&node, &sib)
#define NODE_SET_CHILD(node, child)        node_set_child(&node, &child)

enum node_type {
    T_UNDEFINED = 0,
    T_NIL,
    T_BOOL,
    T_INT, T_LONG, T_CHAR,
    T_UINT, T_ULONG,
    T_DOUBLE, T_LONGDOUBLE,
    T_EXPR,
    T_LIST, T_VECTOR,
    T_FUNCTION, T_POINTER,
};

union node_data {
    int i;            /* T_INT */
    long l;           /* T_LONG */
    char c;           /* T_CHAR, T_LIST, T_VECTOR */
    unsigned int ui;  /* T_UINT */
    unsigned long ul; /* T_ULONG */
    double d;         /* T_DOUBLE */
    long double ld;   /* T_LONGDOUBLE */
    char *s;          /* T_EXPR, T_FUNCTION */
    struct node *n;   /* T_POINTER */
};

struct node {
    enum node_type type;  /* node type */
    union node_data data; /* node data */
    struct node *sibling, /* sibling node pointer */
                *child;   /* child node pointer */
};

enum node_type node_get_type(struct node *);
union node_data node_get_data(struct node *);
struct node *node_get_sibling(struct node *);
struct node *node_get_child(struct node *);

void node_init(struct node *);
void node_set(struct node *, enum node_type, union node_data);
void node_set_sibling(struct node *, struct node *);
void node_set_child(struct node *, struct node *);

#endif

