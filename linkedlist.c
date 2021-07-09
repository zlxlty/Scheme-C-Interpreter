#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "linkedlist.h"
#include "talloc.h"

// Create a new NULL_TYPE value node.
// As the tail of a list
Value *makeNull() {
    Value *newNode = talloc(sizeof(Value)); 
    newNode->type = NULL_TYPE;
    return newNode;
}

// Create a new CONS_TYPE value node.
// Usage: head = cons(value, head);
Value *cons(Value *newCar, Value *newCdr) {
    Value *newCons = talloc(sizeof(Value));
    newCons->type = CONS_TYPE;
    newCons->c.car = newCar;
    newCons->c.cdr = newCdr;

    return newCons;
}

// Display the contents of the linked list to the screen in some kind of
// Format as (1, 2, 3, )
void display(Value *list) {
    if (list == NULL) {
        return;
    }

    Value *curr = list;
    printf("(");
    
    while(!isNull(curr)){
        assert(curr->type == CONS_TYPE && "Error (display): not pointing to a cons type");     
        switch(car(curr)->type){
            case INT_TYPE:
                printf("%i, ", car(curr)->i);
                break;
            case DOUBLE_TYPE:
                printf("%lf, ", car(curr)->d);
                break;
            case STR_TYPE:
                printf("%s, ", car(curr)->s);
                break;
            default: //NULL_TYPE and CONS_TYPE
                break;
        }
        curr = cdr(curr);
    }
    printf(")\n");
}

// Return a new list that is the reverse of the one that is passed in. All
// content within the list should be duplicated; 
// memory whatsoever between the original list and the new one.there should be no shared
//
// FAQ: What if there are nested lists inside that list?
// ANS: There won't be for this assignment. There will be later, but that will
// be after we've got an easier way of managing memory.
Value *reverse(Value *list) {
    assert(list != NULL && "Error (car): input list is NULL");    

    Value *newHead = makeNull();

    for (Value *curr = list; curr->type != NULL_TYPE; curr=cdr(curr)) {
        // switch(car(curr)->type){
        //     case INT_TYPE:
        //         newCar->i = car(curr)->i;
        //         newCar->type = INT_TYPE;
        //         break;
        //     case DOUBLE_TYPE:
        //         newCar->d = car(curr)->d;
        //         newCar->type = DOUBLE_TYPE;
        //         break;
        //     case STR_TYPE:
        //         newCar->s = talloc(sizeof(char)*(strlen(car(curr)->s) + 1));
        //         strcpy(newCar->s, car(curr)->s); //avoid sharing memories in strings
        //         newCar->type = STR_TYPE;
        //         break;
        //     default: //NULL_TYPE and CONS_TYPE
        //         break;
        // }
        newHead = cons(car(curr), newHead);
    }

    return newHead;
}

// Utility

// Get car value. Use assertions to make sure that this is a legitimate operation.
Value *car(Value *list) {
    assert(list != NULL && "Error (car): input list is NULL");
    //assert(list->type == CONS_TYPE && "Error (car): not pointing to a cons type");
    return list->c.car;
}

// Get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list) {
    assert(list != NULL && "Error (cdr): input list is NULL");
    assert(list->type == CONS_TYPE && "Error (cdr): not pointing to a cons type");
    return list->c.cdr;
}

// Check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value){
    assert(value != NULL && "Error (isNull): input list is NULL");
    return value->type == NULL_TYPE;
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value){
    assert(value != NULL && "Error (length): input list is NULL");
    int count = 0; 
    
    for (Value *curr = value; curr->type != NULL_TYPE; curr = cdr(curr)) {
        assert(curr->type == CONS_TYPE && "Error (length): not pointing to a cons type");
        count++;
    }

    return count;
}