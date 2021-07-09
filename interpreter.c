#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "linkedlist.h"
#include "parser.h"
#include "talloc.h"
#include "tokenizer.h"
#include "value.h"
#include "interpreter.h"

//Helper Functions
//look up the value of the symbol in the frame
Value *lookUpSymbol(Value *tree, Frame *frame);
//evaluate if expression
Value *evalIf(Value *args, Frame *frame);
//evaluate let&let* expression
Value *evalLet(Value *args, Frame *frame, Frame *(*frameOperation)(Value *, Frame *));
//evaluate letrec expression
Value *evalLetrec(Value *args, Frame *frame);
//evaluate Define expression
Value *evalDefine(Value *args, Frame *frame);
//evaluate Lambda expression
Value *evalLambda(Value *args, Frame *frame);
//evaluate set! expression
Value *evalSet(Value *args, Frame *frame);
//evaluate set! expression
Value *evalBegin(Value *args, Frame *frame);
//evaluate and expression
Value *evalAnd(Value *args, Frame *frame);
//evaluate or expression
Value *evalOr(Value *args, Frame *frame);
//evaluate cond expression
Value *evalCond(Value *args, Frame *frame);

//create a new frame and add binding to the let expression
Frame *createFrame(Value *bindingHead, Frame *parentFrame);
//print the evaluation result
void printEvalResult(Value *result);
//apply the user define functions
Value *apply(Value *function, Value *args);
//add a new binding to the frame
void addBinding(Value *key, Value *value, Frame *frame);
//create frame for let*
Frame *createLinkedFrames(Value *bindingsHead, Frame *parentFrame);

// Potential bug in struct Value *
void bindPrimitiveFn(char *name, Value *(*function)(Value *), Frame *frame);
//primitiveFunctions
Value *primitivePlus(Value *args);
Value *primitiveMinus(Value *args);
Value *primitiveNull(Value *args);
Value *primitiveCar(Value *args);
Value *primitiveCdr(Value *args);
Value *primitiveCons(Value *args);
Value *primitiveBigger(Value *args);
Value *primitiveSmaller(Value *args);
Value *primitiveEqual(Value *args);
Value *primitiveMultiple(Value *args);
Value *primitiveDivide(Value *args);
Value *primitiveModulo(Value *args);

void interpret(Value *tree) {
    Frame *topLevel = talloc(sizeof(Frame));
    topLevel->bindings = makeNull();
    topLevel->parent = NULL;

    // bind primitive functions
    bindPrimitiveFn("+", primitivePlus, topLevel);
    bindPrimitiveFn("-", primitiveMinus, topLevel);
    bindPrimitiveFn("*", primitiveMultiple, topLevel);
    bindPrimitiveFn("/", primitiveDivide, topLevel);
    bindPrimitiveFn("<", primitiveSmaller, topLevel);
    bindPrimitiveFn(">", primitiveBigger, topLevel);
    bindPrimitiveFn("=", primitiveEqual, topLevel);
    bindPrimitiveFn("modulo", primitiveModulo, topLevel);
    bindPrimitiveFn("null?", primitiveNull, topLevel);
    bindPrimitiveFn("car", primitiveCar, topLevel);
    bindPrimitiveFn("cdr", primitiveCdr, topLevel);
    bindPrimitiveFn("cons", primitiveCons, topLevel);
    

    for (Value *curr = tree; curr->type == CONS_TYPE; curr = cdr(curr)) {
        if (car(curr)->type == SINGLEQUOTE_TYPE){
            curr = cdr(curr);
            printEvalResult(car(curr));
        }else{
            Value *result = eval(car(curr), topLevel);
            printEvalResult(result);
        }
    }
}

Value *eval(Value *tree, Frame *frame) {
    switch (tree->type)  {
        case INT_TYPE:
            return tree;

        case DOUBLE_TYPE:
            return tree;

        case STR_TYPE:
            return tree;
    
        case BOOL_TYPE:
            return tree;
     
        case SYMBOL_TYPE: {
            Value *result = lookUpSymbol(tree, frame);
            // If symbol's value cannot be found
            if (result == NULL) {
                printf("Evaluation error: variable that is not bound in the current frame or any of its ancestors\n");
                texit(0);
            } else {
                return result;
            }
        }

        case CONS_TYPE: {
            Value *first = car(tree);
            Value *args = cdr(tree);
            
            if (!strcmp(first->s, "quote") || first->type == SINGLEQUOTE_TYPE){
                if (length(args) != 1){
                    printf("Evaluation error: quote only allows one parameter\n");
                    texit(0);
                }
                
                return car(args);                

            } else if (first->type != SYMBOL_TYPE && first->type != CONS_TYPE) {
                // Sanity and error checking on first
                printf("Evaluation error: first item is not symbol type in s-expression\n");
                texit(0);

            } else if (!strcmp(first->s, "if")) {
                if (length(args) != 3) {
                    printf("Evaluation error: if expression must have 3 arguments\n");
                    texit(0);
                }
                return evalIf(args,frame);
            
            } else if (!strcmp(first->s, "let")) {
                if (length(args) < 2) {
                    printf("Evaluation error: let expression must have 2 arguments\n");
                    texit(0);
                }
                
                return evalLet(args, frame, createFrame);
                
            } else if (!strcmp(first->s, "let*")) {
                if (length(args) < 2) {
                    printf("Evaluation error: let* expression must have 2 arguments\n");
                    texit(0);
                }
                
                return evalLet(args, frame, createLinkedFrames);
                
            } else if (!strcmp(first->s, "letrec")) {
                if (length(args) < 2) {
                    printf("Evaluation error: letrec expression must have 2 arguments\n");
                    texit(0);
                }
                
                return evalLetrec(args, frame);
                
            } else if (!strcmp(first->s, "set!")) {
                if (length(args) != 2) {
                    printf("Evaluation error: set! expression must have 2 arguments\n");
                    texit(0);
                }
                
                return evalSet(args, frame);
                
            } else if (!strcmp(first->s, "begin")) {
                                
                return evalBegin(args, frame);
                
            } else if (!strcmp(first->s, "display")){
                if (length(args) != 1){
                    printf("Evaluation error: display only allows one parameter\n");
                    texit(0);
                }
                
                return eval(car(args), frame);

            } else if (!strcmp(first->s, "define")){
                if (length(args) != 2) {
                    printf("Evaluation error: define only allows two parameters\n");
                    texit(0);
                }
                
                return evalDefine(args, frame);

            } else if (!strcmp(first->s, "lambda")){
                if (length(args) != 2) {
                    printf("Evaluation error: lambda only allows two parameters\n");
                    texit(0);
                }
                
                return evalLambda(args, frame);

            } else if (!strcmp(first->s, "and")){
                
                return evalAnd(args, frame);

            } else if (!strcmp(first->s, "or")){

                return evalOr(args, frame);

            } else if (!strcmp(first->s, "cond")){
                if (length(args) == 0){
                    printf("Evaluation error: cond needs at least one parameter\n");
                    texit(0);
                }
                
                return evalCond(args, frame);

            } else {
                Value *currProcedure = eval(first, frame);
                //args should be a flat list
                Value *arguments = makeNull();
                for (Value *currArg = args; currArg->type != NULL_TYPE; currArg = cdr(currArg)){
                    arguments = cons(eval(car(currArg), frame), arguments);
                }

                if (currProcedure->type == CLOSURE_TYPE) {
                    return apply(currProcedure, reverse(arguments));
                } else if (currProcedure->type == PRIMITIVE_TYPE){
                    return (currProcedure->primFn)(reverse(arguments));
                } else {
                    printf("Evaluation error: undefined procedure\n");
                    texit(0);
                }
            }
            break;
        }

        default:
            break;
    }    
    return NULL;
}

Value *lookUpSymbol(Value *tree, Frame *frame) {
    for (Frame *currFrame = frame; currFrame != NULL; currFrame = currFrame->parent) {
        for (Value *bindingsHead = currFrame->bindings; bindingsHead->type == CONS_TYPE; bindingsHead = cdr(bindingsHead)) {
            Value *bindingCons = car(bindingsHead);
            // car(bindingCons) is the key cdr(bindingCons) is the value
            if (!strcmp(car(bindingCons)->s, tree->s)) {
                return cdr(bindingCons);
            }
        }
    }
    return NULL;
}

Value *evalIf(Value *args, Frame *frame) {
    Value *condition = eval(car(args), frame);
    
    if (condition->type != BOOL_TYPE){
        printf("Evaluation error: condition of an if expression must be a boolean\n");
        texit(0);
    }

    if (condition->i == 1){
        return eval(car(cdr(args)), frame);
    } else {
        return eval(car(cdr(cdr(args))), frame);
    }
}

Value *evalLet(Value *args, Frame *frame, Frame *(*frameOperation)(Value *, Frame *)){
    Value *result = NULL;
    if (car(args)->type == CONS_TYPE || car(args)->type == NULL_TYPE){

        if (car(args)->type == CONS_TYPE) {
            frame = frameOperation(car(args), frame);
        }

        for (Value *currExpr = cdr(args); currExpr->type != NULL_TYPE; currExpr = cdr(currExpr)) {
            result = eval(car(currExpr), frame);
        }
    } else {
        printf("Evaluation error: the first argument of let must be a nested list\n");
        texit(0);
    }
    return result;
}

Value *evalLetrec(Value *args, Frame *frame) {
    Value *expressions = makeNull();
    Value *bindingsHead = car(args);
    Value *result = NULL;
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->bindings = makeNull();
    newFrame->parent = frame;

    //error checking
    if (car(args)->type != CONS_TYPE && car(args)->type != NULL_TYPE){
        printf("Evaluation error: the first argument of letrec must be a nested list\n");
        texit(0);
    }

    //check e1 to en
    for (Value *curr = bindingsHead; curr->type != NULL_TYPE; curr = cdr(curr)){
        expressions = cons(eval(car(cdr(car(curr))), newFrame), expressions);
    }

    //create new frame
    expressions = reverse(expressions);

    //add bindings
    for (Value *curr = bindingsHead; curr->type != NULL_TYPE; curr = cdr(curr), expressions = cdr(expressions)){
        Value *key = car(car(curr));
        if (key->type != SYMBOL_TYPE){
            printf("Evaluation error: left side of a let pair doesn't have a variable.\n");
            texit(0);
        } else if (lookUpSymbol(key, newFrame) == NULL){
            addBinding(key, car(expressions), newFrame);
            
        } else {
            printf("Evaluation error: duplicate variable in let\n");
            texit(0);
        }
    }

    //evaluate body
    for (Value *currExpr = cdr(args); currExpr->type != NULL_TYPE; currExpr = cdr(currExpr)) {
        result = eval(car(currExpr), newFrame);
        //printf("Here\n");
    }

    return result;
}

Value *evalDefine(Value *args, Frame *frame){
    Value *name = car(args);
    Value *body = car(cdr(args));

    //Capstone Work 1
    //special define (with lambda) type
    //e.g. (define (myfunction x y z) (+ x y z))
    if (name->type == CONS_TYPE){
        name = car(car(args));
        body = cons(cdr(car(args)), body);
        Value *lambda = talloc(sizeof(Value));
        lambda->type = SYMBOL_TYPE;
        lambda->s = "lambda";
        body = cons(lambda, body);
    } else if (name->type != SYMBOL_TYPE) {
        printf("Evaluation error: first argument in define must be symbol type \n");
        texit(0);
    }

    addBinding(name, eval(body, frame), frame);

    Value *returnValue = talloc(sizeof(Value));
    returnValue->type = VOID_TYPE;
    
    return returnValue;
}

Value *evalLambda(Value *args, Frame *frame){
    Value *newClosure = talloc(sizeof(Value));
    newClosure->type = CLOSURE_TYPE;

    if (car(args)->type == CONS_TYPE) {
        for (Value *curr = car(args); curr->type != NULL_TYPE; curr = cdr(curr)) {
            if (car(curr)->type != SYMBOL_TYPE) {
                printf("Evaluation error: first parameter in lambda must be symbol type\n");
                texit(0);
            }
            for (Value *rest = cdr(curr); rest->type != NULL_TYPE; rest = cdr(rest)){
                if (!strcmp(car(curr)->s, car(rest)->s)){
                    printf("Evaluation error: duplicate identifier in lambda\n");
                    texit(0);
                }
            }
        }

        //paramNames is a flat linkedlist
        newClosure->closure.paramNames = car(args);
    //Capstone work 2
    } else if (car(args)->type == SYMBOL_TYPE) {
        newClosure->closure.paramNames = car(args);
    } else if (car(args)->type == NULL_TYPE) {
        newClosure->closure.paramNames = makeNull();
    } else {
        printf("Evaluation error: first parameter in lambda must be symbol type or cons type\n");
        texit(0);
    }

    newClosure->closure.fnBody = car(cdr(args));
    newClosure->closure.frame = frame;

    return newClosure;
}

Value *evalSet(Value *args, Frame *frame){
    for (Frame *currFrame = frame; currFrame != NULL; currFrame = currFrame->parent) {
        for (Value *bindingsHead = currFrame->bindings; bindingsHead->type == CONS_TYPE; bindingsHead = cdr(bindingsHead)) {
            Value *bindingCons = car(bindingsHead);
            // car(bindingCons) is the key cdr(bindingCons) is the value
            if (!strcmp(car(bindingCons)->s, car(args)->s)) {
                bindingCons->c.cdr = eval(car(cdr(args)), frame);
            }
        }
    }

    Value *returnValue = talloc(sizeof(Value));
    returnValue->type = VOID_TYPE;
    
    return returnValue;
}

Value *evalBegin(Value *args, Frame *frame){
    Value *returnValue = talloc(sizeof(Value));
    returnValue->type = VOID_TYPE;

    for (Value *curr = args; curr->type != NULL_TYPE; curr = cdr(curr)) {
        returnValue = eval(car(curr), frame);
    }
    
    return returnValue;
}

Value *evalAnd(Value *args, Frame *frame){
    Value *returnValue = talloc(sizeof(Value));
    returnValue->type = BOOL_TYPE;
    returnValue->i = 1;

    for (Value *curr = args; curr->type != NULL_TYPE; curr = cdr(curr)) {
        returnValue = eval(car(curr), frame);
        if (returnValue->type == BOOL_TYPE && returnValue->i == 0) {
            return returnValue;
        }
    }

    return returnValue;
}

Value *evalOr(Value *args, Frame *frame){
    Value *returnValue = talloc(sizeof(Value));
    returnValue->type = BOOL_TYPE;
    returnValue->i = 0;
    
    for (Value *curr = args; curr->type != NULL_TYPE; curr = cdr(curr)) {
        returnValue = eval(car(curr), frame);
        if (!(returnValue->type == BOOL_TYPE && returnValue->i == 0)) {
            return returnValue;
        }
    }
    
    return returnValue;
}

Value *evalCond(Value *args, Frame *frame){
    Value *returnValue = talloc(sizeof(Value));
    returnValue->type = VOID_TYPE;
    
    for (Value *curr = args; curr->type != NULL_TYPE; curr = cdr(curr)) {
        if (car(curr)->type != CONS_TYPE) {
            printf("Evaluation error [cond]: argument of cond must be cons type\n");
            texit(0);
        }
        
        if (car(car(curr))->type == SYMBOL_TYPE) {
            if (!strcmp(car(car(curr))->s, "else")) {
                returnValue = eval(car(cdr(car(curr))), frame);
                break;
            } else {
                printf("Evaluation error [cond]: unknown symbol in condition\n");
                texit(0);
            }
        } else {
            
            Value *condResult = eval(car(car(curr)), frame);
            if (condResult->type == BOOL_TYPE){
                if (condResult->i == 1){
                    returnValue = eval(car(cdr(car(curr))), frame);
                    break;
                }
            } else {
                printf("Evaluation error [cond]: incorrect type in condition\n");
                texit(0);
            }
        }
    }

    return returnValue;
}

Frame *createLinkedFrames(Value *bindingsHead, Frame *parentFrame){
    Frame *preFrame = parentFrame;
    for (Value *currBinding = bindingsHead; currBinding->type == CONS_TYPE; currBinding = cdr(currBinding)){
        if (car(currBinding)->type != CONS_TYPE) {
            printf("Evaluation error: bad format in let\n");
            texit(0);
        }
        if (length(car(currBinding)) != 2){
            printf("Evaluation error: invalid number of keys and bindings\n");
            texit(0);
        }

        Frame *newFrame = talloc(sizeof(Frame));
        newFrame->bindings = makeNull();
        newFrame->parent = NULL;

        Value *newBinding = talloc(sizeof(Value));
        newBinding->type = CONS_TYPE;
        
        Value *key = car(car(currBinding));
        Value *value = car(cdr(car(currBinding)));

        if (key->type != SYMBOL_TYPE){
            printf("Evaluation error: left side of a let pair doesn't have a variable.\n");
            texit(0);
        } else if (lookUpSymbol(key, newFrame) == NULL){
            newBinding->c.car = key;
            newBinding->c.cdr = eval(value, preFrame);
        } else {
            printf("Evaluation error: duplicate variable in let\n");
            texit(0);
        }
        
        newFrame->bindings = cons(newBinding, newFrame->bindings);
        newFrame->parent = preFrame;
        preFrame = newFrame;
    }
    
    return preFrame;
}

Frame *createFrame(Value *bindingsHead, Frame *parentFrame){
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->bindings = makeNull();
    newFrame->parent = NULL;
    
    for (Value *currBinding = bindingsHead; currBinding->type == CONS_TYPE; currBinding = cdr(currBinding)){
        if (car(currBinding)->type != CONS_TYPE) {
            printf("Evaluation error: bad format in let\n");
            texit(0);
        }
        if (length(car(currBinding)) != 2){
            printf("Evaluation error: invalid number of keys and bindings\n");
            texit(0);
        }

        Value *newBinding = talloc(sizeof(Value));
        newBinding->type = CONS_TYPE;
        
        Value *key = car(car(currBinding));
        Value *value = car(cdr(car(currBinding)));

        if (key->type != SYMBOL_TYPE){
            printf("Evaluation error: left side of a let pair doesn't have a variable.\n");
            texit(0);
        } else if (lookUpSymbol(key, newFrame) == NULL){
            newBinding->c.car = key;
            newBinding->c.cdr = eval(value, parentFrame);
        } else {
            printf("Evaluation error: duplicate variable in let\n");
            texit(0);
        }
        
        newFrame->bindings = cons(newBinding, newFrame->bindings);
    }
    
    newFrame->parent = parentFrame;
    return newFrame;
}

// function and args should already be evaluated
Value *apply(Value *function, Value *args){
    if (function->type != CLOSURE_TYPE) {
        printf("Evaluation error: function must be CLOSURE_TYPE\n");
        texit(0);
    }

    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->bindings = makeNull();
    newFrame->parent = function->closure.frame;

    // Capstone work 2
    if (function->closure.paramNames->type == SYMBOL_TYPE) {
        addBinding((function->closure.paramNames), args, newFrame);
        return eval(function->closure.fnBody, newFrame);
    }

    if (length(function->closure.paramNames) != length(args)) {
        printf("Evaluation error: incurrent number of arguments of procedure\n");
        texit(0);
    }

    for (Value *key = function->closure.paramNames; key->type != NULL_TYPE; key = cdr(key), args = cdr(args)){
        addBinding(car(key), car(args), newFrame);
    }

    return eval(function->closure.fnBody, newFrame);
}

void addBinding(Value *key, Value *value, Frame *frame){
    Value *newBinding = talloc(sizeof(Value));
    newBinding->type = CONS_TYPE;
    newBinding->c.car = key;
    newBinding->c.cdr = value;
    frame->bindings = cons(newBinding, frame->bindings);
}

void bindPrimitiveFn(char *name, Value *(*function)(struct Value *), Frame *frame) {
    Value *newName = talloc(sizeof(Value));
    newName->type = SYMBOL_TYPE;
    newName->s = name;

    Value *newFunction = talloc(sizeof(Value));
    newFunction->type = PRIMITIVE_TYPE;
    newFunction->primFn = function;

    addBinding(newName, newFunction, frame);
}

Value *primitivePlus(Value *args){
    Value *returnValue = talloc(sizeof(Value));
    if (args->type == NULL_TYPE){
        returnValue->type = INT_TYPE;
        returnValue->i = 0;
    } else if (car(args)->type == INT_TYPE || car(args)->type == DOUBLE_TYPE) {
        int i = 0;
        double d = 0.0;
        for (Value *argument = args; argument->type != NULL_TYPE; argument = cdr(argument)) {
            if (car(argument)->type == DOUBLE_TYPE){
                d += car(argument)->d;
            } else if (car(argument)->type == INT_TYPE) {
                i += car(argument)->i;
            } else {
                printf("Evaluation error: incurrent type for plus argument\n");
                texit(0);
            }
        }
        if (d != 0.0){
            returnValue->type = DOUBLE_TYPE;
            returnValue->d = d + i;
        } else {
            returnValue->type = INT_TYPE;
            returnValue->i = i;
        }
    }
    return returnValue;
}

Value *primitiveMinus(Value *args){
    if (args->type == NULL_TYPE){
        printf("Evaluation error: incorrect number of minus argument\n");
        texit(0);
    }

    Value *returnValue = talloc(sizeof(Value));

    double subtractValue = 0.0;
    int hasDouble = 0;
    Value *remainValues = args;

    if (length(args) > 1) {
        if (car(args)->type == INT_TYPE) {
            subtractValue = car(args)->i;
            remainValues = cdr(args);
        } else if (car(args)->type == DOUBLE_TYPE) {
            subtractValue = car(args)->d;
            remainValues = cdr(args);
            hasDouble = 1;
        } else {
            printf("Evaluation error: incorrect type for minus argument\n");
            texit(0);
        }
    }
    
    for (Value *argument = remainValues; argument->type != NULL_TYPE; argument = cdr(argument)) {
        if (car(argument)->type == DOUBLE_TYPE){
            subtractValue -= car(argument)->d;
            hasDouble = 1;
        } else if (car(argument)->type == INT_TYPE) {
            subtractValue -= car(argument)->i;
        } else {
            printf("Evaluation error: incorrect type for plus argument\n");
            texit(0);
        }
    }

    
    if (hasDouble){
        returnValue->type = DOUBLE_TYPE;
        returnValue->d = subtractValue;
    } else {
        returnValue->type = INT_TYPE;
        returnValue->i = subtractValue;
    }
    
    return returnValue;
}

Value *primitiveNull(Value *args){
    if (length(args) != 1){
        printf("Evaluation error: incurrent number for null? argument\n");
        texit(0);
    }

    Value *returnValue = talloc(sizeof(Value));
    returnValue->type = BOOL_TYPE;
    returnValue->i = isNull(car(args));

    return returnValue;
}

Value *primitiveCar(Value *args){
    if (length(args) != 1){
        printf("Evaluation error: incurrent number for car argument\n");
        texit(0);
    } else if (car(args)->type != CONS_TYPE) {
        printf("Evaluation error: car argument must be a CONS_TYPE\n");
        texit(0);
    }

    return car(car(args));
}

Value *primitiveCdr(Value* args) {
    if (length(args) != 1){
        printf("Evaluation error [cdr]: incurrent number for cdr argument\n");
        texit(0);
    } else if (car(args)->type != CONS_TYPE) {
        printf("Evaluation error: cdr argument must be a CONS_TYPE\n");
        texit(0);
    }

    return cdr(car(args));
}

Value *primitiveCons(Value *args){
    if (length(args) != 2) {
        printf("Evaluation error [cons]: incurrent number for cons argument\n");
        texit(0);
    }

    Value *newConscell = talloc(sizeof(Value));
    newConscell->type = CONS_TYPE;
    newConscell->c.car = car(args);
    newConscell->c.cdr = car(cdr(args));

    return newConscell;
}

Value *primitiveBigger(Value *args){
    Value *returnValue = talloc(sizeof(Value));
    returnValue->type = BOOL_TYPE;
    returnValue->i = 1;

    if (length(args) < 2) {
        return returnValue;
    }

    double preNumber;
    double thisNumber;
    
    if (car(args)->type == INT_TYPE){
        preNumber = car(args)->i;
    } else if (car(args)->type == DOUBLE_TYPE){
        preNumber = car(args)->d;
    } else{
        printf("Evaluation error [>]: incorrect type for argument\n");
        texit(0);
    }

    for (Value *argument = cdr(args); argument->type != NULL_TYPE; argument = cdr(argument)) {
        if (car(argument)->type == INT_TYPE){
            thisNumber = car(argument)->i;
        } else if (car(argument)->type == DOUBLE_TYPE){
            thisNumber = car(argument)->d;
        } else {
            printf("Evaluation error [>]: incorrect type for argument\n");
            texit(0);
        }

        if (thisNumber >= preNumber){
            returnValue->i = 0;
            break;
        }

        preNumber = thisNumber;
    }

    return returnValue;
}

Value *primitiveSmaller(Value *args){
    Value *returnValue = talloc(sizeof(Value));
    returnValue->type = BOOL_TYPE;
    returnValue->i = 1;

    if (length(args) < 2) {
        return returnValue;
    }

    double preNumber;
    double thisNumber;
    
    if (car(args)->type == INT_TYPE){
        preNumber = car(args)->i;
    } else if (car(args)->type == DOUBLE_TYPE){
        preNumber = car(args)->d;
    } else{
        printf("Evaluation error [<]: incorrect type for argument\n");
        texit(0);
    }

    for (Value *argument = cdr(args); argument->type != NULL_TYPE; argument = cdr(argument)) {
        if (car(argument)->type == INT_TYPE){
            thisNumber = car(argument)->i;
        } else if (car(argument)->type == DOUBLE_TYPE){
            thisNumber = car(argument)->d;
        } else {
            printf("Evaluation error [<]: incorrect type for argument\n");
            texit(0);
        }

        if (thisNumber <= preNumber){
            returnValue->i = 0;
            break;
        }

        preNumber = thisNumber;
    }

    return returnValue;
}

Value *primitiveEqual(Value *args){
    Value *returnValue = talloc(sizeof(Value));
    returnValue->type = BOOL_TYPE;
    returnValue->i = 1;

    if (length(args) < 2) {
        return returnValue;
    }

    double preNumber;
    double thisNumber;
    
    if (car(args)->type == INT_TYPE){
        preNumber = car(args)->i;
    } else if (car(args)->type == DOUBLE_TYPE){
        preNumber = car(args)->d;
    } else{
        printf("Evaluation error [=]: incorrect type for argument\n");
        texit(0);
    }

    for (Value *argument = cdr(args); argument->type != NULL_TYPE; argument = cdr(argument)) {
        if (car(argument)->type == INT_TYPE){
            thisNumber = car(argument)->i;
        } else if (car(argument)->type == DOUBLE_TYPE){
            thisNumber = car(argument)->d;
        } else {
            printf("Evaluation error [=]: incorrect type for argument\n");
            texit(0);
        }

        if (thisNumber != preNumber){
            returnValue->i = 0;
            break;
        }

        preNumber = thisNumber;
    }

    return returnValue;
}

Value *primitiveMultiple(Value *args){
    Value *returnValue = talloc(sizeof(Value));
    if (length(args) < 2){
        printf("Evaluation error [*]: incorrect number of arguments\n");
        texit(0);
    } else if (car(args)->type == INT_TYPE || car(args)->type == DOUBLE_TYPE) {
        int hasDouble = 0;
        double d = 1.0;

        for (Value *argument = args; argument->type != NULL_TYPE; argument = cdr(argument)) {
            if (car(argument)->type == DOUBLE_TYPE){
                d *= car(argument)->d;
                hasDouble = 1;
            } else if (car(argument)->type == INT_TYPE) {
                d *= car(argument)->i;
            } else {
                printf("Evaluation error: incurrent type for plus argument\n");
                texit(0);
            }
        }
        
        if (hasDouble){
            returnValue->type = DOUBLE_TYPE;
            returnValue->d = d;
        } else {
            returnValue->type = INT_TYPE;
            returnValue->i = d;
        }
    }
    return returnValue;
}

Value *primitiveDivide(Value *args){
    Value *returnValue = talloc(sizeof(Value));
    if (length(args) != 2){
        printf("Evaluation error [/]: incorrect number of arguments\n");
        texit(0);
    }

    double divident, divisor;
    int hasInt = 0;
    if (car(args)->type == INT_TYPE) {
        divident = car(args)->i;
        hasInt++;
    } else if (car(args)->type == DOUBLE_TYPE) {
        divident = car(args)->d;
    } else {
        printf("Evaluation error [/]: incorrect type for divident\n");
        texit(0);
    }
    
    if (car(cdr(args))->type == INT_TYPE) {
        divisor = car(cdr(args))->i;
        hasInt++;
    } else if (car(cdr(args))->type == DOUBLE_TYPE) {
        divisor = car(cdr(args))->d;
    } else {
        printf("Evaluation error [/]: incorrect type for divisor\n");
        texit(0);
    }

    if (divisor == 0) {
        printf("Evaluation error [/]: divisor can't be zero\n");
        texit(0);
    }

    if (hasInt == 2 && ((int)divident % (int)divisor) == 0) {
        returnValue->type = INT_TYPE;
        returnValue->i = divident / divisor;
    } else {
        returnValue->type = DOUBLE_TYPE;
        returnValue->d = divident / divisor;
    }
    
    return returnValue;
}

Value *primitiveModulo(Value *args){
    if (length(args) != 2){
        printf("Evaluation error [modulo]: incorrect number of arguments\n");
        texit(0);
    } else if (car(args)->type != INT_TYPE || car(cdr(args))->type != INT_TYPE) {
        printf("Evaluation error [modulo]: incorrect type for arguments\n");
        texit(0);
    }
    
    Value *returnValue = talloc(sizeof(Value));
    returnValue->type = INT_TYPE;
    returnValue->i = car(args)->i % car(cdr(args))->i;

    return returnValue;   
}

void printEvalResult(Value *result) {
    switch(result->type) {
        case INT_TYPE:
            printf("%d\n", result->i);
            break;

        case DOUBLE_TYPE:
            printf("%lf\n", result->d);
            break;

        case STR_TYPE:
            printf("\"%s\"\n", result->s);
            break;

        case SYMBOL_TYPE:
            printf("%s\n", result->s);
            break;
            
        case BOOL_TYPE:
            if (result->i == 0) {
                printf("#f\n");
            } else {
                printf("#t\n");
            }
            break;

        case CONS_TYPE:{
            printf("(");
            for (Value *curr = result; curr->type != NULL_TYPE; curr = cdr(curr)) {
                if (car(curr)->type == SINGLEQUOTE_TYPE) {
                    printf("(quote\n");
                    curr = cdr(curr);
                    printEvalResult(car(curr));
                    printf(")\n");        
                } else {
                    printEvalResult(car(curr));
                }
                if (cdr(curr)->type != CONS_TYPE && cdr(curr)->type != NULL_TYPE) {
                    printf(" . ");
                    printEvalResult(cdr(curr));
                    break;
                }
            }
            printf(")\n");
            break;
        }

        case NULL_TYPE:
            printf("()\n");
            break;

        case VOID_TYPE:
            break;
        
        case CLOSURE_TYPE:
            printf("#<procedure>");
            break;
        
        default:
            printf("Evaluation error: unknown type for evalution result\n");
            texit(0);
    }
}