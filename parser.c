#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "linkedlist.h"
#include "parser.h"
#include "talloc.h"
#include "tokenizer.h"
#include "value.h"

Value *addToParseTree(Value *tree, int *depth, Value* token);

Value *parse(Value *tokens) {

    Value *tree = makeNull();
    int depth = 0;

    Value *current = tokens;
    assert(current != NULL && "Error (parse): null pointer");

    while (current->type != NULL_TYPE) {
        Value *token = car(current);
        tree = addToParseTree(tree, &depth, token);
        current = cdr(current);
    }
    if (depth != 0) {
        printf("Syntax error: not enough close parentheses\n");
        texit(0);
    }

    return reverse(tree);
}

void printTree(Value *tree) {

    for (Value *curr = tree; curr->type == CONS_TYPE; curr = cdr(curr)) {
        switch(car(curr)->type){
            case INT_TYPE:
                printf("%i", car(curr)->i);
                break;

            case DOUBLE_TYPE:
                printf("%.2lf", car(curr)->d);
                break;

            case DOT_TYPE:
                printf("%s", car(curr)->s);
                break;
            
            case SINGLEQUOTE_TYPE:
                printf("\'");
                break;

            case BOOL_TYPE:
                if(car(curr)->i == 0){
                    printf("#f");
                }else{
                    printf("#t");
                }
                break;
                
            case SYMBOL_TYPE:
                printf("%s", car(curr)->s);
                break;

            case STR_TYPE:
                printf("\"%s\"", car(curr)->s);
                break;

            case CONS_TYPE:
                printf("(");
                printTree(car(curr));
                printf(")");

                break;

            case NULL_TYPE:
                printf("()");
                break;

            default:
                break;

        }
        if (cdr(curr)->type != NULL_TYPE) {
            printf(" ");
        }
    }

}

Value *addToParseTree(Value *tree, int *depth, Value* token) {
    if (token == NULL) {
        printf("Syntax error: Empty token");
        texit(0);
    }

    if (token->type == OPEN_TYPE) {
        (*depth)++;
    }

    Value *curr = tree;

    if (token->type != CLOSE_TYPE) {
        curr = cons(token, curr);
        return curr;
    } else {
        Value *newTree = makeNull();
        while (curr->type != NULL_TYPE && car(curr)->type != OPEN_TYPE) {
                newTree = cons(car(curr), newTree);
                curr = cdr(curr);
        }
        if (curr->type == NULL_TYPE) {
            printf("Syntax error: too many close parentheses\n");
            texit(0);
        }
        // Now curr is an OPEN_TYPE
        curr = cons(newTree, cdr(curr));
        (*depth)--;
        return curr;
    }

}