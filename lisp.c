/**
 * clisp
 * =====
 * another lisp interpreter in C
 *
 * author: Violet McClure
 * date: Feb 2020
 * version: 0.0
 */

#include <stdio.h>
#include <stdlib.h>

#include <regex.h>
#include <sys/types.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "node.h"
#include "vector.h"

int main(void);
int READ(char prompt[], struct vector *);
struct node* EVAL(struct vector *);
void PRINT(struct vector *);
int tokenize(char *, struct vector *);
int furl(struct vector *, struct vector *);

int
main(void)
{
    size_t index, endindex;
    char prompt[101];
    struct vector forest, tree;
    vector_init(&forest, sizeof(struct vector));
    snprintf(prompt, sizeof(prompt), "%s", "λ> ");
    while (!READ(prompt, &forest)) {
        for (index = 0, endindex = vector_size(&forest); index < endindex; ++index) {
            vector_remove(&forest, 0, &tree);
            vector_free(&tree);
            PRINT(&tree);
        }
    }
    vector_free(&forest);
    return 0;
}

int
READ(char prompt[], struct vector *forest)
{
    char *line;
    int err = 0;
    line = readline(prompt);
    if (line) {
        err = tokenize(line, forest);
        if (err)
            fputs("Fatal Error during tokenization\n", stderr);
        else {
            add_history(line);
            free(line);
        }
    } else {
        err = 1;
    }
    return err;
}

void
printNode(struct node *node) {
    if (!node) {
        printf("nil");
        return;
    }
    switch (node->type) {
        case T_NIL:
            printf("nil");
            break;
        case T_BOOL:
            printf("%s", node->data.c ? "true" : "false");
            break;
        case T_INT:
            printf("%d", node->data.i);
            break;
        case T_LONG:
            printf("%ld", node->data.l);
            break;
        case T_CHAR:
            printf("'");
            switch (node->data.c) {
                case '\a': printf("\\a");  break;
                case '\b': printf("\\b");  break;
                case   27: printf("\\e");  break;
                case '\f': printf("\\f");  break;
                case '\n': printf("\\n");  break;
                case '\r': printf("\\r");  break;
                case '\t': printf("\\t");  break;
                case '\v': printf("\\v");  break;
                case '\\': printf("\\\\"); break;
                case '\'': printf("\\'");  break;
                default  : printf("%c", node->data.c);
            }
            printf("'");
            break;
        case T_LIST:
        case T_VECTOR:
            printf("%c", node->data.c);
            break;
        case T_UINT:
            printf("%u", node->data.ui);
            break;
        case T_ULONG:
            printf("%lu", node->data.ul);
            break;
        case T_DOUBLE:
            printf("%f", node->data.d);
            break;
        case T_LONGDOUBLE:
            printf("%Lf", node->data.ld);
            break;
        case T_POINTER:
            printf("%f", node->data.d);
            break;
        case T_EXPR:
            printf("#<%s expression>", node->data.s);
            break;
        case T_FUNCTION:
            printf("#<%s function>", node->data.s);
            break;
        case T_UNDEFINED:
            printf("#<undefined>");
    }
}

void
PRINT(struct vector *tree)
{
    struct node node, *nextnode;
    struct vector parents;
    vector_init(&parents, sizeof(struct node));
    nextnode = vector_get(tree, 0);
    node = *nextnode;
    char space = 0, endbrace = 0;
    while (nextnode && node.type != T_UNDEFINED) {
        endbrace = (node.type == T_LIST && node.data.c == ')')
            || (node.type == T_VECTOR && node.data.c == ']');
        if (space && node.sibling && !endbrace)
            printf(" ");
        space = node.type != T_LIST && node.type != T_VECTOR;
        printNode(&node);
        if (endbrace && node.sibling
                && !((node.sibling->type == T_LIST && node.sibling->data.c == ')')
                    || (node.sibling->type == T_VECTOR && node.sibling->data.c == ']')))
            printf(" ");
        nextnode = node.child;
        if (nextnode) {
            vector_push(&parents, &node);
        } else {
            nextnode = node.sibling;
            if (!nextnode && vector_size(&parents)) {
                vector_pop(&parents, nextnode);
                if (nextnode->type == T_LIST)
                    printf(")");
                else if (nextnode->type == T_VECTOR)
                    printf("]");
            }
        }
        node = *nextnode;
    }
    printf("\n");
    vector_free(&parents);
}

int
reduceSign(char *signStr, size_t signLen)
{
    int sign = 1;
    size_t ind = 0;
    while (ind++ < signLen) {
        if (signStr[ind] == '-')
            sign *= -1;
    }
    return sign;
}

void
escapeChar(char *chr)
{
    switch (*chr) {
        case 'a': *chr = '\a'; break;
        case 'b': *chr = '\b'; break;
        case 'e': *chr =   27; break;
        case 'f': *chr = '\f'; break;
        case 'n': *chr = '\n'; break;
        case 'r': *chr = '\r'; break;
        case 't': *chr = '\t'; break;
        case 'v': *chr = '\v'; break;
    }
}

long double
s2ld(char *s, char *end)
{
    long double result = 0, fact = 1;
    int afterDecimal, num;
    for (afterDecimal = 0; s < end; s++){
        if (*s == '.'){
            afterDecimal = 1; 
            continue;
        };
        num = *s - '0';
        if (num >= 0 && num <= 9){
            if (afterDecimal) fact /= 10.0l;
            result = result * 10.0l + (long double)num;
        };
    };
    return result * fact;
}

unsigned long long
s2ull(char *start, char *end, int base)
{
    unsigned long long result = 0; char c;
    while (start < end) {
        c = tolower(*(start++));
        result = result * base + (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10);
    }
    return result;
}

int
tokenize(char *expr, struct vector *forest)
{
    struct vector tree;
    char *subexpr, *startchar, *endchar;
    int state, base, sign, flag;
    size_t index, endindex;
    struct node node;
    regex_t regexComment     , regexSpace        , regexNilBool, regexFloat       , regexDecimal,
            regexBinary      , regexOctal        , regexHex    , regexPostNumError, regexSymbol ,
            regexSingleSymbol, regexSpecialSymbol, regexChar   , regexString;
    regmatch_t matches[5], errMatches[1];
    regcomp(&regexSpace        , "^\\s+"                           , REG_EXTENDED);
    regcomp(&regexComment      , "^;.*"                            , REG_EXTENDED);
    regcomp(&regexNilBool      , "^(nil|true|false)"               , 0);
    regcomp(&regexFloat        , "^([+-]*)([0-9]*\\.[0-9]+)(d?)"   , REG_EXTENDED);
    regcomp(&regexDecimal      , "^([+-]*)([0-9]+)(u?)(l?)"        , REG_EXTENDED);
    regcomp(&regexBinary       , "^([+-]*)0[bB]([01]+)(u?)(l?)"    , REG_EXTENDED);
    regcomp(&regexOctal        , "^([+-]*)0[oO]?([0-7]+)(u?)(l?)"  , REG_EXTENDED);
    regcomp(&regexHex          , "^([+-]*)0[xX]([0-9a-f]+)(u?)(l?)", REG_EXTENDED);
    regcomp(&regexPostNumError , "^[a-zA-Z0-9_.]*"                 , REG_EXTENDED);
    regcomp(&regexSymbol       , "^[a-z_][a-z_0-9!@#']*"           , REG_ICASE);
    regcomp(&regexSingleSymbol , "^[][()]"                         , REG_EXTENDED);
    regcomp(&regexSpecialSymbol, "^[][+*=|/~()<>?!@#$%^&*=-]+"     , REG_EXTENDED);
    regcomp(&regexChar         , "^'(\\\\)?(.)'"                   , REG_EXTENDED);
    regcomp(&regexString       , "^\"([^\"\\]|\\\\.)*\""           , REG_EXTENDED);
    vector_init(&tree, sizeof(struct node));
    while (*expr != '\0' && *expr != '\n') {
        /* state -> 0:error, -1:skip, 1: nil or bool, 2:float, 3:decimal,
         *          4:binary, 5:octal, 6:hex, -> 7:expr, 8:char, 9:str */
        state = 0;
        if      (regexec(&regexSpace        , expr, 1, matches, 0) == 0) state = -1;
        else if (regexec(&regexComment      , expr, 1, matches, 0) == 0) state = -1;
        else if (regexec(&regexNilBool      , expr, 2, matches, 0) == 0) state = 1;
        else if (regexec(&regexFloat        , expr, 4, matches, 0) == 0) state = 2;
        else if (regexec(&regexDecimal      , expr, 5, matches, 0) == 0) state = 3;
        else if (regexec(&regexBinary       , expr, 5, matches, 0) == 0) state = 4;
        else if (regexec(&regexOctal        , expr, 5, matches, 0) == 0) state = 5;
        else if (regexec(&regexHex          , expr, 5, matches, 0) == 0) state = 6;
        else if (regexec(&regexSymbol       , expr, 1, matches, 0) == 0) state = 7;
        else if (regexec(&regexSingleSymbol , expr, 1, matches, 0) == 0) state = 7;
        else if (regexec(&regexSpecialSymbol, expr, 1, matches, 0) == 0) state = 7;
        else if (regexec(&regexChar         , expr, 1, matches, 0) == 0) state = 8;
        else if (regexec(&regexString       , expr, 1, matches, 0) == 0) state = 9;
        else                                                               state = 0;
        if (!state) {
            vector_free(&tree);
            fprintf(stderr, "Fatal Error: Invalid state for rest of expression: “%s”", expr);
            return 1;
        }
        if (state > 1 && state < 7) {
            flag = regexec(&regexPostNumError, expr + matches[0].rm_eo, 1, errMatches, 0);
            if (flag == 0 && errMatches[0].rm_eo) {
                fprintf(stderr, "Fatal Error: Invalid suffix for number: \"%.*s\"\n",
                        errMatches[0].rm_eo, expr);
                return 2;
            }
        }
        if (state > 0) {
            if (state == 1) {
                if (*expr == 'n') {
                    node.type = T_NIL;
                    node.data.c = 0;
                } else {
                    node.type = T_BOOL;
                    if (*expr == 't')
                        node.data.c = 1;
                    else
                        node.data.c = 0;
                }
            } else if (state == 2) {
                sign = reduceSign(expr, matches[1].rm_eo);
                startchar = expr + matches[2].rm_so;
                endchar   = expr + matches[2].rm_eo;
                if (matches[3].rm_eo > matches[3].rm_so) {
                    node.type = T_LONGDOUBLE;
                    node.data.ld = s2ld(startchar, endchar) * sign;
                } else {
                    node.type = T_DOUBLE;
                    node.data.d = (double)s2ld(startchar, endchar);
                }
            } else if (state < 7) {
                sign = reduceSign(expr, matches[1].rm_eo);
                if (matches[3].rm_eo > matches[3].rm_so)
                    flag = 1; /* not signed */
                else
                    flag = 0; /* signed */
                if (matches[1].rm_eo > matches[1].rm_so && flag)
                    fprintf(stderr, "Warning: unsigned number has prefixed sign: \"%.*s\"",
                            matches[0].rm_eo, expr);
                switch (state) {
                    case 3:  base = 10; break;
                    case 4:  base =  2; break;
                    case 5:  base =  8; break;
                    case 6:  base = 16; break;
                    default: base = 1; /* impossible */
                }
                startchar = expr + matches[2].rm_so;
                endchar   = expr + matches[2].rm_eo;
                if (matches[4].rm_eo > matches[4].rm_so) { /* long */
                    if (flag) { /* unsigned */
                        node.type = T_ULONG;
                        node.data.ul = (unsigned long)s2ull(startchar, endchar, base);
                    } else { /* signed */
                        node.type = T_LONG;
                        node.data.l = (long)s2ull(startchar, endchar, base);
                    }
                } else { /* int */
                    if (flag) { /* unsigned */
                        node.type = T_UINT;
                        node.data.ui = (unsigned int)s2ull(startchar, endchar, base);
                    } else { /* signed */
                        node.type = T_INT;
                        node.data.i = (int)s2ull(startchar, endchar, base);
                    }
                }
            } else if (state == 7) {
                switch (*expr) {
                    case '(':
                    case ')':
                        node.type = T_LIST;
                        node.data.c = *expr;
                        break;
                    case '[':
                    case ']':
                        node.type = T_VECTOR;
                        node.data.c = *expr;
                        break;
                    default:
                        node.type = T_EXPR;
                        node.data.s = malloc((matches[0].rm_eo + 1) * sizeof(char));
                        strncpy(node.data.s, expr, matches[0].rm_eo);
                        node.data.s[matches[0].rm_eo] = '\0';
                }
            } else if (state == 8) {
                node.type = T_CHAR;
                node.data.c = *(expr + 1);
                if (matches[1].rm_eo > matches[1].rm_so)
                    escapeChar(&node.data.c);
            } else if (state == 9) {
                node.type = T_VECTOR;
                node.data.c = '[';
                vector_push(&tree, &node);
                subexpr = expr + 1;
                index = 1;
                endindex = matches[0].rm_eo - 1;
                flag = 0;
                node.type = T_CHAR;
                while (index++ < endindex) {
                    if (!flag && *subexpr == '\\') {
                        flag = 1;
                    } else {
                        node.data.c = *subexpr;
                        if (flag) {
                            escapeChar(&node.data.c);
                            flag = 0;
                        }
                        vector_push(&tree, &node);
                    }
                }
                node.type = T_VECTOR;
                node.data.c = ']';
            }
            vector_push(&tree, &node);
        }
        expr += matches[0].rm_eo;
    }
    furl(forest, &tree);
    return 0;
}

int
furl(struct vector *forest, struct vector *tree)
{
    struct node node, *nodeptr, parent;
    struct vector parents, ftree;
    vector_init(&parents, sizeof(struct node));
    vector_init(&ftree, sizeof(struct node));
    while (vector_size(tree)) {
        vector_remove(tree, 0, &node);
        vector_push(&ftree, &node);
        nodeptr = (struct node*)vector_get(&free, 0);
        if (node.type == T_LIST || node.type == T_VECTOR) {
            vector_push(&parents, &node);
        }
    }
    /* while (vector_size(tree)) { */
    /*     vector_remove(vec, 0, &node); */
    /*     /1* printNode(&node); *1/ */
    /*     if (node.type == T_LIST || node.type == T_VECTOR) { */
    /*             vector_push(&parents, &node); */
    /*             node.child = node.sibling; */
    /*             node.sibling = NULL; */
    /*             node = *node.child; */
    /*         } else { */
    /*             if (!vector_size(&parents)) { */
    /*                 fprintf(stderr, "Fatal Error: Unmatched closing '%c'.\n", node.data.c); */
    /*                 vector_free(&parents); */
    /*                 return 10; */
    /*             } */
    /*             vector_pop(&parents, &parent); */
    /*             if (node.sibling && node.sibling->type && !vector_size(&parents)) { */
    /*                 vector_push(forest, tree); */
    /*                 tree_add_siblings(tree, node); */
    /*             } */
    /*             parent.sibling = *node.sibling; */
    /*             node.sibling = NULL; */
    /*             node = *parent.sibling; */
    /*         } */
    /*     } else { */
    /*         node = node.sibling; */
    /*     } */
    /* } */
    /* if (vector_size(&parents)) { */
    /*     fputs("Fatal Error: Unmatched opening brace\n", stderr); */
    /*     vector_free(&parents); */
    /*     return 11; */
    /* } */
    vector_free(&parents);
    return 0;
}

