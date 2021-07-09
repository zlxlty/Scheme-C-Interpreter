#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"
#include "tokenizer.h"

//helper functions

//check if the input string is a integer
int isuinteger(char *str);
//check if the input string is a decimal
int isudecimal(char *str);
//check if the input string is a symbol
int issymbol(char *str);
//check if the input char is an initial
int isinitial(char chr);
//check if the input char is a subsequent
int issubsequent(char chr);


// Read all of the input from stdin, and return a linked list consisting of the
// tokens.
Value *tokenize(){
    char charRead;
    Value *list = makeNull();
    charRead = (char)fgetc(stdin);
    char buffer[300]; 
    int totalToken = 0;

    while (charRead != EOF) {

        if (charRead == '.') {
            
            charRead = fgetc(stdin);
            if (totalToken == 0 || charRead == EOF) {
                printf("Syntax error: untokenizeable (unvalid . position)\n");
                texit(0);

            } else if (isspace(charRead)) {
                Value *openNode = talloc(sizeof(Value));
                openNode->type = DOT_TYPE;
                openNode->s = talloc(sizeof(char)*2);
                strcpy(openNode->s, ".");
                list = cons(openNode, list);
                
            } else {
                // before: .9432; after: charRead = 9, '.' is lost, 432 remains

                //unget will insert number after current position
                ungetc(charRead, stdin); //restore the number at current position, 9432 remains
                charRead = '.'; //set previous char to be '.'
            }
        }

        if (charRead == '(') {
            //open parenthesis
            Value *openNode = talloc(sizeof(Value));
            openNode->type = OPEN_TYPE;
            openNode->s = talloc(sizeof(char)*2);
            strcpy(openNode->s,"(");
            list = cons(openNode, list);
            
        } else if (charRead == ')') {
            //close parenthesis
            Value *closeNode = talloc(sizeof(Value));
            closeNode->type = CLOSE_TYPE;
            closeNode->s = talloc(2);
            strcpy(closeNode->s,")");
            list = cons(closeNode, list);

        } else if (charRead == '[') {
            //open bracket
            Value *closeNode = talloc(sizeof(Value));
            closeNode->type = OPENBRACKET_TYPE;
            closeNode->s = talloc(2);
            strcpy(closeNode->s,"[");
            list = cons(closeNode, list);

        } else if (charRead == ']') {
            //close bracket
            Value *closeNode = talloc(sizeof(Value));
            closeNode->type = CLOSEBRACKET_TYPE;
            closeNode->s = talloc(2);
            strcpy(closeNode->s,"]");
            list = cons(closeNode, list);

        } else if (charRead == '#') {
            //test whether the token is #t or #f
            charRead = (char)fgetc(stdin);
            int boolValue;
            if (charRead == 't') {
                boolValue = 1;
            } else if (charRead == 'f') {
                boolValue = 0;
            } else {
                //error message
                printf("Syntax error: untokenizeable (unvalid char after #)\n");
                texit(0);
            }
            //cons newNode
            Value *newNode = talloc(sizeof(Value));
            newNode->type = BOOL_TYPE;
            newNode->i = boolValue;
            list = cons(newNode, list);

        } else if (charRead == '"'){  // string type
            memset((void*)buffer, '\0', sizeof(char)*300);
            charRead = (char)fgetc(stdin);

            while (charRead != '"') {

                if (strlen(buffer) >= 300) {
                    printf("Memory error: maximum string length reached\n");
                    texit(0);
                }

                strncat(buffer, &charRead, 1);

                if (charRead == '\\') {  // Read whatever is after "\" into the string
                    charRead = (char)fgetc(stdin);
                    strncat(buffer, &charRead, 1);
                }

                charRead = (char)fgetc(stdin);
            }

            char *token = talloc((strlen(buffer)+1)*sizeof(char));
            strcpy(token, buffer);

            Value *newNode = talloc(sizeof(Value));
            newNode->type = STR_TYPE;
            newNode->s = token;
            list = cons(newNode, list);

        } else if (charRead == '\''){
            char nextChar = fgetc(stdin);
            if (isspace(nextChar)) {
                //error message
                printf("Syntax error: untokenizeable (Invalid token ' position)\n");
                texit(0);
            }

            ungetc(nextChar, stdin);

            Value *closeNode = talloc(sizeof(Value));
            closeNode->type = SINGLEQUOTE_TYPE;
            closeNode->s = talloc(2);
            strcpy(closeNode->s,"\'");
            list = cons(closeNode, list);

        } else if (charRead == ';'){  // comment case
            while (charRead != '\n' && charRead != '\r' && charRead != EOF){
                charRead = (char)fgetc(stdin);
            }

        } else {
            while (isspace(charRead)) {
                charRead = (char)fgetc(stdin);
            }



            //find the int/double/symbol token
            memset((void*)buffer, '\0', sizeof(char)*300);
            while (!isspace(charRead) && charRead != EOF) {
                if (charRead == '#' || charRead == ')' || charRead == ']') { //add ]
                    ungetc(charRead, stdin);
                    break;
                }

                strncat(buffer, &charRead, 1);
                charRead = (char)fgetc(stdin);
            }


            //give buffer and clear up its content
            char *token = talloc((strlen(buffer)+1)*sizeof(char));
            strcpy(token, buffer);

            
            //test and insert result into the linked list
            Value *newNode = talloc(sizeof(Value));
            if (isuinteger(token)) {
                //test int
                newNode->type = INT_TYPE;
                newNode->i = (int)strtol(token, NULL, 10);
            } else if (isudecimal(token)) {
                //test double
                newNode->type = DOUBLE_TYPE;
                newNode->d = strtod(token, NULL);
            } else if (issymbol(token)) {
                newNode->type = SYMBOL_TYPE;
                newNode->s = token;
            } else {
                //error message
                printf("Syntax error: untokenizeable (Invalid token %s)\n", token);
                texit(0);
            }
            list = cons(newNode, list); 
        }
        
        //strip all space
        do {
            charRead = (char)fgetc(stdin);
        } while (isspace(charRead));

        totalToken++;
    }


    Value *revList = reverse(list);
    return revList;
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list) {
    
    for (Value *curr = list; curr->type != NULL_TYPE; curr = cdr(curr)) {
        switch(car(curr)->type){
            case INT_TYPE:
                printf("%i:integer\n", car(curr)->i);
                break;

            case DOUBLE_TYPE:
                printf("%lf:double\n", car(curr)->d);
                break;

            case OPEN_TYPE:
                printf("%s:open\n", car(curr)->s);
                break;

            case CLOSE_TYPE:
                printf("%s:close\n", car(curr)->s);
                break;

            case OPENBRACKET_TYPE:
                printf("%s:openbracket\n", car(curr)->s);
                break;

            case CLOSEBRACKET_TYPE:
                printf("%s:closebracket\n", car(curr)->s);
                break;

            case DOT_TYPE:
                printf("%s:dot\n", car(curr)->s);
                break;
            
            case SINGLEQUOTE_TYPE:
                printf("':single quote\n");
                break;

            case BOOL_TYPE:
                if(car(curr)->i == 0){
                    printf("#f:boolean\n");
                }else{
                    printf("#t:boolean\n");
                }
                break;
                
            case SYMBOL_TYPE:
                printf("%s:symbol\n", car(curr)->s);
                break;

            case STR_TYPE:
                printf("\"%s\":string\n", car(curr)->s);
                break;

            default:
                break;
        }
    }
}


//check if the input string is a integer
int isuinteger(char *str) {
    int index = 0;
    if (str[0] == '+' || str[0] == '-') {
        if (strlen(str) == 1) {
            return 0;
        }
        
        index = 1;
    }
    for (; index < strlen(str); index++) {
        if (!isdigit(str[index])){
            return 0;
        }
    }
    return 1;
}

//check if the input string is a decimal
int isudecimal(char *str) {
    int index = 0;
    int hasDecimal = 0;

    if (str[0] == '+' || str[0] == '-'){
        if (strlen(str) == 1) {
            return 0;
        }
        index = 1;
    }
    for (; index < strlen(str); index++){
        if (str[index] == '.' && hasDecimal != 1){
            hasDecimal = 1;
        } else if (!isdigit(str[index])){
            return 0;
        }
    }

    return hasDecimal;
}

int issymbol(char *str) {
    if (strlen(str) == 1) {
        char chr = str[0];
        if (isinitial(chr) || chr == '+' || chr == '-') {
            return 1;
        }
        return 0;
    }
    
    //first check subsequent elements are valid
    for (int index = 1; index < strlen(str); index ++) {
        if (!issubsequent(str[index])){
            return 0;
        }
    }

    return isinitial(str[0]); //check whether the first char is an initial
}

int isinitial(char chr) {
    char specialChar[15] = "!$%&*/:<=>?~_^";
    for (int i = 0; i < 14; i++){
        if(chr == specialChar[i]){
            return 1;
        }
    }
    return isalpha(chr);
}

int issubsequent(char chr) {
    if(isinitial(chr) || isdigit(chr) ||
       chr == '.' || chr == '+' || chr == '-'){
            return 1;
    }
    return 0;
}