
#include "node.h"

enum node_type
node_get_type(struct node *node) {
    return node->type;
}

union node_data
node_get_data(struct node *node) {
    return node->data;
}

struct node *
node_get_sibling(struct node *node)
{
    return node->sibling;
}

struct node *
node_get_child(struct node *node)
{
    return node->child;
}

void
node_init(struct node *node)
{
    node->type = T_UNDEFINED;
    node->sibling = NULL;
    node->child = NULL;
}

void
node_set(struct node *node, enum node_type type, union node_data data)
{
    node->type = type;
    node->data = data;
}

void
node_set_sibling(struct node *node, struct node *sibling)
{
    node->sibling = sibling;
}

void
node_set_child(struct node *node, struct node *child)
{
    node->child = child;
}

