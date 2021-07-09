#include <stdlib.h>
#include <stdio.h>
#include "value.h"
#include "talloc.h"

//helper function

//Memory count helper function
static int tallocCountHelper(Value *curr);

static Value *head = NULL;

// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size){
    //create a new null head node
    if (head == NULL) {
        head = (Value*)malloc(sizeof(Value));
        head->type = NULL_TYPE;
    }
    
    //create newNode to store the pointer of talloced space
    Value *newNode = malloc(sizeof(Value));
    newNode->type = PTR_TYPE;
    newNode->p = malloc(size);

    //cons head and newNode
    Value *newCons = malloc(sizeof(Value));
    newCons->type = CONS_TYPE;
    newCons->c.car = newNode;
    newCons->c.cdr = head;
    
    //move head to the front
    head = newCons;

    return newNode->p;
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree(){
    if (head == NULL) {
        return;
    }

    Value *next;
    Value *curr = head; 

    while (curr->type != NULL_TYPE){
        //curr->c.car->type must be PTR_TYPE
        free(curr->c.car->p);
        next = curr->c.cdr;
        free(curr->c.car);
        free(curr);
        curr = next;
    }

    free(curr); //free the final nullValue
    head = NULL;
}

// 
// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status){
    tfree();
    exit(status);
}

int tallocMemoryCount(){
    return tallocCountHelper(head);
}

static int tallocCountHelper(Value *curr) {
    if (curr == NULL) {
        return 0;
    }

    if (curr->type == NULL_TYPE) {
        return sizeof(*curr);
    }

    int currSize = sizeof(*curr)+sizeof(*(curr->c.car))+sizeof(*(curr->c.car->p));

    return currSize + tallocCountHelper(curr->c.cdr);
}

