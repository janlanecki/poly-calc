/** @file
   Implementation of the polynomial calculator class

   @author Jan Łanecki <jl385826@students.mimuw.edu.pl>
   @copyright University of Warsaw, Poland
   @date 2017-05-25
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include "poly.h"
#include "stack_poly.h"
#include "utils.h"

/** number of characters in the name of the longest command (IS_COEFF) */
#define MAX_COMMAND_LENGTH 8
/** number of commands */
#define NUM_OF_COMMANDS 15
/** character which opens a sequence in the notation of polynomials */
#define POLY_OPENING_SEPARATOR '('
/** character which closes a sequence in the notation of polynomials */
#define POLY_CLOSING_SEPARATOR ')'
/** character which separates coefficient and exponential in
  * the notation of polynomials */
#define POLY_COEFF_EXP_SEPARATOR ','
/** character which separates words in command names */
#define COMMAND_WORDS_SEPARATOR '_'
/** character which separates command names from parameters */
#define COMMAND_PARAM_SEPARATOR ' '
#define NO_ERROR 0
#define ERROR_HANDLED 1
#define ERROR_UNHANDLED 2

/** Value returned by function, which checks the content of a given line */
#define VAL_IF_POLY 0
/** Value returned by function, which checks the content of a given line */
#define VAL_IF_COMMAND 1
/** Value returned by function, which checks the content of a given line */
#define VAL_IF_EOF (-1)
/** value if the parsed number is inside a monomial */
#define NUM_IN_MONO 0
/** value if the parsed number is at the end of a line */
#define NUM_LAST 1
/** value if the parsed number is a coefficient and a part of a command */
#define COEFF_IN_COMMAND 2
/** value if the parsed number is an exponential and is a part of a command */
#define EXP_IN_COMMAND 3
/** value if the parsed number is a count in command */
#define COUNT_IN_COMMAND 4
/** Number by which the size of the array in the ParseTermsHelper
  *  function is multiplicated */
#define POLY_SIZE_MULTIPLICATION 2
/** Starting size of the array in the ParseTermsHelper function */
#define POLY_ARR_STARTING_SIZE 2

/**
 * Enumerates calculator commands.
 */
typedef enum CommandId {
    ZERO_ID, IS_COEFF_ID, IS_ZERO_ID, CLONE_ID, ADD_ID, MUL_ID, NEG_ID, 
    SUB_ID, IS_EQ_ID, DEG_ID, DEG_BY_ID, AT_ID, PRINT_ID, POP_ID, COMPOSE_ID
} CommandId;

/**
 * Structure for commands and possible parameters.
 */
typedef struct command_and_params {
    CommandId id; ///< id of a command
    poly_exp_t degByParam; ///< parameter of the DEG_BY command
    poly_coeff_t atParam; ///< parameter of the AT command
    unsigned composeParam; ///< parameter of the COMPOSE command
}  CommandAndParam;

/**
 * Array with the names of the commands.
 */
const char *arrayOfCommands[NUM_OF_COMMANDS] = {
    "ZERO", "IS_COEFF", "IS_ZERO", "CLONE", "ADD", "MUL", "NEG", "SUB", "IS_EQ",
    "DEG", "DEG_BY", "AT", "PRINT", "POP", "COMPOSE"
};

void UnderflowErrorMsg(int lineCount) {
    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineCount);
}

void ParsingErrorMsg(int lineCount, int columnCount) {
    fprintf(stderr, "ERROR %d %d\n", lineCount, columnCount);
}

void WrongCommandErrorMsg(int lineCount) {
    fprintf(stderr, "ERROR %d WRONG COMMAND\n", lineCount);
}

void WrongValueErrorMsg(int lineCount) {
    fprintf(stderr, "ERROR %d WRONG VALUE\n", lineCount);
}

void WrongVariableErrorMsg(int lineCount) {
    fprintf(stderr, "ERROR %d WRONG VARIABLE\n", lineCount);
}

void WrongCountErrorMsg(int lineCount) {
    fprintf(stderr, "ERROR %d WRONG COUNT\n", lineCount);
}

/**
 * Checks if the character is a digit.
 * @param c
 * @return 
 */
bool IsDigit(int c) {
    return (c >= '0' && c <= '9');
}

/**
 * Checks if the number overflows if muliplied 10 times and increased
 * by the value of the digit.
 * @param number
 * @param digit
 * @return 
 */
bool CheckIfUnsignedIntOverflows(int number, int digit) {
    if ((number > UINT_MAX / 10) || (number * 10 > UINT_MAX - digit)) {
        return true;
    }
    return false;
}

/**
 * Evaluates an exponential from stdin.
 * @param overflows
 * @param c
 * @param columnCount
 * @return 
 */
poly_exp_t CalculateUnsigned(bool *overflows, int *c, int *columnCount) {
    unsigned number = 0;
    unsigned ascii = '0';
    while (IsDigit(*c) && !*overflows) {
        *overflows = CheckIfUnsignedIntOverflows(number, *c - ascii);
        if (!*overflows) {
            number *= 10;
            number += *c - ascii;
            *c = getchar();
            (*columnCount)++;
        }
    }
    return number;
}

/**
 * Evaluates an unsigned variable from stdin.
 * @param inside
 * @param exp
 * @param lineCount
 * @param columnCount
 * @return 
 */
bool ParseUnsigned(int inside, unsigned *exp, int lineCount, int *columnCount) {
    int lastChar, c;
    lastChar = c = getchar();
    (*columnCount)++;
    if (!IsDigit(c)) {
        if (inside == EXP_IN_COMMAND) {
            WrongVariableErrorMsg(lineCount);
        }
        else if (inside == COUNT_IN_COMMAND) {
            WrongCountErrorMsg(lineCount);
        }
        else {
            ParsingErrorMsg(lineCount, *columnCount);
        }
        ungetc(lastChar, stdin); (*columnCount)--;
        return true;
    }
    bool overflows = false, result;
    poly_exp_t number = CalculateUnsigned(&overflows, &c, columnCount);
    lastChar = c;
    if (overflows) {
        if (inside == EXP_IN_COMMAND) {
            WrongVariableErrorMsg(lineCount);
        }
        else if (inside == COUNT_IN_COMMAND) {
            WrongCountErrorMsg(lineCount);
        }
        else if (inside == NUM_IN_MONO) {
            ParsingErrorMsg(lineCount, *columnCount);
        }
        result = true;
    }
    else if (inside == NUM_IN_MONO && c != POLY_CLOSING_SEPARATOR) {
        ParsingErrorMsg(lineCount, *columnCount); 
        result = true;
    }
    else if (c != '\n' && c != EOF &&
            (inside == EXP_IN_COMMAND || inside == COUNT_IN_COMMAND)) {
            if (inside == EXP_IN_COMMAND) {
                WrongVariableErrorMsg(lineCount);
            }
            else if (inside == COUNT_IN_COMMAND) {
                WrongCountErrorMsg(lineCount);
            }
        result = true;
    }
    else {
        *exp = number;
        result = false;
    }
    ungetc(lastChar, stdin); (*columnCount)--;
    return result;
}

/**
 * Evaluates an exponential inside a polynomial.
 * @param exp
 * @param lineCount
 * @param columnCount
 * @return 
 */
bool ParseExpInPoly(poly_exp_t *exp, int lineCount, int *columnCount) {
    return ParseUnsigned(NUM_IN_MONO, exp, lineCount, columnCount);
}

/**
 * Evaluates an exponential inside a command.
 * @param exp
 * @param lineCount
 * @param columnCount
 * @return 
 */
bool ParseExpInCommand(poly_exp_t *exp, int lineCount, int *columnCount) {
    return ParseUnsigned(EXP_IN_COMMAND, exp, lineCount, columnCount);
}

/**
 * Evaluates a count inside a command.
 * @param count
 * @param lineCount
 * @param columnCount
 * @return 
 */
bool ParseCount(unsigned *count, int lineCount, int *columnCount) {
    return ParseUnsigned(COUNT_IN_COMMAND, count, lineCount, columnCount);
}

/**
 * Checks if the number overflows if muliplied 10 times and increased
 * by the value of the digit.
 * @param number
 * @param digit
 * @return 
 */
bool CheckIfLongOverflows(long number, long digit) {
    if ((number >= 0 && number > LONG_MAX / 10 || 
         number < 0 && number < LONG_MIN / 10) ||
        (number >= 0 && number * 10 > LONG_MAX - digit ||
         number <= 0 && number * 10 < LONG_MIN + digit)) {
        return true;
    }
    return false;
}

/**
 * Evaluates a coefficient from stdin.
 * @param overflows
 * @param negative
 * @param c
 * @param columnCount
 * @return 
 */
poly_coeff_t CalculateCoeff(bool *overflows, bool negative, int *c, 
                            int *columnCount) {
    poly_coeff_t number = 0;
    poly_coeff_t ascii = '0';
    while (IsDigit(*c) && !*overflows) {
        *overflows = CheckIfLongOverflows(number, *c - ascii);
        if (!*overflows) {
            if (negative) {
                number *= 10;
                number -= *c - ascii;
            }
            else {
                number *= 10;
                number += *c - ascii;
            }
            *c = getchar();
            (*columnCount)++;
        }
    }
    return number;
}

/**
 * Evaluates a coefficient.
 * @param inside
 * @param coeff
 * @param lineCount
 * @param columnCount
 * @return 
 */
bool ParseCoeff(int inside, poly_coeff_t *coeff, int lineCount, 
                       int *columnCount) {
    int lastChar, c;
    lastChar = c = getchar();
    (*columnCount)++;
    bool negative = false;
    if (c == '-') {
        negative = true;
        lastChar = c = getchar();
        (*columnCount)++;
    }
    if (!IsDigit(c)) {
        if (inside == COEFF_IN_COMMAND) {
            WrongValueErrorMsg(lineCount);
        }
        else {
            ParsingErrorMsg(lineCount, *columnCount);
        }
        ungetc(lastChar, stdin); (*columnCount)--;
        return true;
    }
    
    bool overflows = false, result;
    poly_coeff_t number = CalculateCoeff(&overflows, negative, &c, columnCount);
    lastChar = c;
    if (overflows) {
        if (inside == COEFF_IN_COMMAND) {
            WrongValueErrorMsg(lineCount);
        }
        else if (inside == NUM_IN_MONO || inside == NUM_LAST) {
            ParsingErrorMsg(lineCount, *columnCount);
        }
        result = true;
    }
    else if (inside == NUM_IN_MONO && c != POLY_COEFF_EXP_SEPARATOR ||
             inside == NUM_LAST && c != '\n' && c != EOF) {
        ParsingErrorMsg(lineCount, *columnCount);
        result = true;
    }
    else if (inside == COEFF_IN_COMMAND && c != '\n' && c != EOF) {
        WrongValueErrorMsg(lineCount);
        result = true;
    }
    else {
        *coeff = number;
        result = false;
    }
    ungetc(lastChar, stdin); (*columnCount)--;
    return result;
}

/**
 * Evaluates a coefficient inside a polynomial.
 * @param coeff
 * @param lineCount
 * @param columnCount
 * @return 
 */
bool ParseCoeffInPoly(poly_coeff_t *coeff, int lineCount, int *columnCount) {
    return ParseCoeff(NUM_IN_MONO, coeff, lineCount, columnCount);
}

/**
 * Evaluates a coefficient at the end of line.
 * @param coeff
 * @param lineCount
 * @param columnCount
 * @return 
 */
bool ParseCoeffAtEnd(poly_coeff_t *coeff, int lineCount, int *columnCount) {
    return ParseCoeff(NUM_LAST, coeff, lineCount, columnCount);
}

/**
 * Evaluates a coefficient in a command.
 * @param coeff
 * @param lineCount
 * @param columnCount
 * @return 
 */
bool ParseCoeffInCommand(poly_coeff_t *coeff, int lineCount, int *columnCount) {
    return ParseCoeff(COEFF_IN_COMMAND, coeff, lineCount, columnCount);
}

/**
 * Fills the buffer with a command name.
 * @param c
 * @param count
 * @param columnCount
 * @param buf
 */
void FillCommandBuffer(int *c, int *count, int *columnCount, char buf[]) {
    while ((*c >= 'A' && *c <= 'Z' || *c == COMMAND_WORDS_SEPARATOR) &&
            *count < MAX_COMMAND_LENGTH) {
        buf[*count] = *c;
        *c = getchar();
        (*count)++;
        (*columnCount)++;
    }
    buf[*count] = '\0';
}

/**
 * Parses a command form stdin.
 * @param cap
 * @param lineCount
 * @return 
 */
bool ParseCommand(CommandAndParam *cap, int lineCount) {
    char buf[MAX_COMMAND_LENGTH + 1];
    int lastChar, c;
    lastChar = c = getchar(); 
    int count = 0, columnCount = 1;
    int commandId = 0;
    bool error = false;
    FillCommandBuffer(&c, &count, &columnCount, buf);
    lastChar = c;
    while (commandId < NUM_OF_COMMANDS &&
           strcmp(buf, arrayOfCommands[commandId]) != 0) {
        commandId++;
    }
    if (commandId < NUM_OF_COMMANDS &&
        strcmp(buf, arrayOfCommands[commandId]) == 0) {
        if ((commandId == DEG_BY_ID || commandId == AT_ID || 
             commandId == COMPOSE_ID) && c != ' ' && c != '\n' && c != EOF) {
            /* If the name of the command isn't divided by whitespace
             * and isn't the end of the line. */
            WrongCommandErrorMsg(lineCount);
            error = true;
        }
        else if (commandId == DEG_BY_ID) {
            if (c == ' ') {  
                poly_exp_t index;
                if (!ParseExpInCommand(&index, lineCount, &columnCount)) {
                    cap->degByParam = index;
                    cap->id = commandId;
                }
                else {
                    error = true;
                }
                lastChar = getchar(); columnCount++;
            }
            else {
                WrongVariableErrorMsg(lineCount);
                error = true;
            }
        }
        else if (commandId == AT_ID) {
            if (c == ' ') {
                poly_coeff_t coeff;
                if (!ParseCoeffInCommand(&coeff, lineCount, &columnCount)) {
                    cap->atParam = coeff;
                    cap->id = commandId;
                }
                else {
                    error = true;
                }
                lastChar = getchar(); columnCount++;
            }
            else {
                WrongValueErrorMsg(lineCount);
                error = true;
            }
        }
        else if (commandId == COMPOSE_ID) {
            if (c == ' ') {
                unsigned count;
                if (!ParseCount(&count, lineCount, &columnCount)) {
                    cap->composeParam = count;
                    cap->id = commandId;
                }
                else {
                    error = true;
                }
            }
            else {
                WrongCountErrorMsg(lineCount);
                error = true;
            }
        }
        else {
            if (c == '\n' || c == EOF) {
                cap->id = commandId;
            }
            else {
                fprintf(stderr, "ERROR %d WRONG COMMAND\n", lineCount);
                error = true;
            }
        }
    }
    else {
        fprintf(stderr, "ERROR %d WRONG COMMAND\n", lineCount);
        error = true;
    }
    ungetc(lastChar, stdin); columnCount--;
    return error;
}

int ParsePolyHelper(Poly *p, int lineCount, int *columnCount);

/**
 * Parses and builds a monomial.
 * @param m
 * @param lineCount
 * @param columnCount
 * @return message about lack of error, handled error or unhandled error
 */
int ParseMonoHelper(Mono *m, int lineCount, int *columnCount) {
    int error = ERROR_UNHANDLED;
    int c, lastChar;
    lastChar = c = getchar();
    (*columnCount)++;
    Poly p;
    if (c == POLY_OPENING_SEPARATOR) {
        ungetc(c, stdin); (*columnCount)--;
        error = ParsePolyHelper(&p, lineCount, columnCount);
        lastChar = getchar(); (*columnCount)++;
        if (error == NO_ERROR){ 
            c = lastChar = getchar();
            (*columnCount)++;
            if (c == POLY_COEFF_EXP_SEPARATOR) {
                error = NO_ERROR;
            }
            else {
                PolyDestroy(&p);
                error = ERROR_UNHANDLED;
            }
        }
    }
    else if (IsDigit(c) || c == '-') {
        poly_coeff_t coeff;
        ungetc(c, stdin);
        (*columnCount)--;
        if (!ParseCoeffInPoly(&coeff, lineCount, columnCount)) {
            p = PolyFromCoeff(coeff);
            error = NO_ERROR;
        }
        else {
            error =  ERROR_HANDLED;
        }
        lastChar = c = getchar(); (*columnCount)++;
    }
    if (error == NO_ERROR) {
        poly_exp_t exp;
        error = ParseExpInPoly(&exp, lineCount, columnCount);
        lastChar = c = getchar(); (*columnCount)++;
        if (error == NO_ERROR) {
            m->exp = exp;
            m->p = p;
            ungetc(lastChar, stdin); (*columnCount)--;
            return NO_ERROR;
        }
        else {
            PolyDestroy(&p);
            error = ERROR_HANDLED;
        }
    }
    ungetc(lastChar, stdin); (*columnCount)--;
    return error;
}

/**
 * Parses and builds a polynomial if it isn't only const polynomial in the line.
 * @param p
 * @param lineCount
 * @param columnCount
 * @return message about lack of error, handled error or unhandled error
 */
int ParsePolyHelper(Poly *p, int lineCount, int *columnCount) {
    int count = 0, size = POLY_ARR_STARTING_SIZE, error = NO_ERROR;
    int lastChar, c;
    lastChar = c = getchar();
    (*columnCount)++;
    bool hasNext = true;
    Mono *arr = calloc(size, sizeof(Mono));
    assert (arr != NULL);
    while (error == NO_ERROR && hasNext) {
        if (c != POLY_OPENING_SEPARATOR) {
            error = ERROR_UNHANDLED;
        }
        else {
            if (count >= size) {
                size *= POLY_SIZE_MULTIPLICATION;
                arr = realloc(arr, size * sizeof(Mono));
                assert(arr != NULL);
            }
            Mono temp;
            error = ParseMonoHelper(&temp, lineCount, columnCount);
            lastChar = getchar(); (*columnCount)++;
            if (error == NO_ERROR) {
                if (!PolyIsZero(&temp.p)) {
                    arr[count] = temp;
                    count++;
                }
                c = getchar();
                (*columnCount)++;
                if (c != '+') {
                    hasNext = false;
                    ungetc(c, stdin);
                    (*columnCount)--;
                }
                else {
                    lastChar = c = getchar();
                    (*columnCount)++;
                }
            }
        }
    }
    if (error == NO_ERROR) {
        *p = PolyAddMonos(count, arr);
    }
    else {
        for (int i = 0; i < count; i++) {
            PolyDestroy(&arr[i].p);
        }
    }
    free(arr);
    ungetc(lastChar, stdin); (*columnCount)--;
    return error;
}

/**
 * Parses and build a polynomial from stdin.
 * @param p
 * @param lineCount
 * @return 0 if incorrect, 1 if correct
 */
bool ParsePoly(Poly *p, int lineCount) {
    int lastChar, c;
    lastChar = c = getchar();
    int columnCount = 1, error, result;
    if (c != POLY_OPENING_SEPARATOR && c != '-' && !IsDigit(c)) {
        ParsingErrorMsg(lineCount, columnCount);
        result = true;
    }
    else if (IsDigit(c) || c == '-') {
        poly_coeff_t coeff;
        ungetc(c, stdin); columnCount--;
        error = ParseCoeffAtEnd(&coeff, lineCount, &columnCount);
        lastChar = getchar(); columnCount++;
        if (!error) {
            *p = PolyFromCoeff(coeff);
            result = false;
        }
        else {
            result = true;
        }
    }
    else {
        ungetc(c, stdin); columnCount--;
        error = ParsePolyHelper(p, lineCount, &columnCount);
        lastChar = c = getchar(); columnCount++;
        if (error == NO_ERROR) {
            lastChar = c = getchar(); columnCount++;
            if (c == EOF || c == '\n') {
                result = false;
            }
            else {
                error = ERROR_UNHANDLED;
            }
        }
        if (error == ERROR_UNHANDLED) {
            ParsingErrorMsg(lineCount, columnCount);
            result = true;
        }
        else if (error == ERROR_HANDLED) {
            result = true;
        }
    }
    ungetc(lastChar, stdin); columnCount--;
    return result;
}

/**
 * Checks if the stack contains more than one polynomial.
 * @param sPtr
 * @return 
 */
bool TwoElementsOnStack(Stack **sPtr) {
    if (!Empty(*sPtr)) {
        Poly p = Pop(sPtr);
        bool result = !Empty(*sPtr);
        Push(sPtr, p);
        return result;
    }
    return false;
}

/**
 * Pushes polynomial equal to zero on the stack.
 * @param sPtr
 */
void PushZero(Stack **sPtr) {
    Push(sPtr, PolyZero());
}

/**
 * Checks if a polynomial on the top of the stack is a coefficient.
 * @param s
 * @return 
 */
bool IsCoeff(Stack *s) {
    if (!Empty(s)) {
        Poly p = Top(s);
        printf("%d\n", PolyIsCoeff(&p));
        return false;
    }
    return true;
}

/**
 * Checks if a polynomial equal to zero is on top of the stack.
 * @param s
 * @return 
 */
bool IsZero(Stack *s) {
    if (!Empty(s)) {
        Poly p = Top(s);
        printf("%d\n", PolyIsZero(&p));
        return false;
    }
    return true;
}

/**
 * Checks if two polynomials on the top of the stack are equal.
 * @param sPtr
 * @return 
 */
bool IsEq(Stack **sPtr) {
    if (TwoElementsOnStack(sPtr)) {
        Poly p = Pop(sPtr);
        Poly q = Top(*sPtr);
        Push(sPtr, p);
        printf("%d\n", PolyIsEq(&p, &q));
        return false;
    }
    return true;
}

/**
 * Puts a copy of the polynomial on the top of the stack on the stack.
 * @param sPtr
 * @return 
 */
bool Clone(Stack **sPtr) {
    if (!Empty(*sPtr)) {
        Poly p = Top(*sPtr);
        Push(sPtr, PolyClone(&p));
        return false;
    }
    return true;
}

/**
 * Adds the two polynomials on the top of the stack.
 * @param sPtr
 * @return 
 */
bool Add(Stack **sPtr) {
    if (TwoElementsOnStack(sPtr)) {
        Poly p = Pop(sPtr);
        Poly q = Pop(sPtr);
        Push(sPtr, PolyAdd(&p, &q));
        return false;
    }
    return true;
}

/**
 * Multiplies the two polynomials on the top of the stack.
 * @param sPtr
 * @return 
 */
bool Mul(Stack **sPtr) {
    if (TwoElementsOnStack(sPtr)) {
        Poly p = Pop(sPtr);
        Poly q = Pop(sPtr);
        Push(sPtr, PolyMul(&p, &q));
        return false;
    }
    return true;
}

/**
 * Negates a polynomial on the top od the stack.
 * @param sPtr
 * @return 
 */
bool Neg(Stack **sPtr) {
    if (!Empty(*sPtr)) {
        Poly p = Pop(sPtr);
        Push(sPtr, PolyNeg(&p));
        PolyDestroy(&p);
        return false;
    }
    return true;
}

/**
 * Subtracts the second from top polynomial from the top polynomial.
 * @param sPtr
 * @return 
 */
bool Sub(Stack **sPtr) {
    if (TwoElementsOnStack(sPtr)) {
        Poly p = Pop(sPtr);
        Poly q = Pop(sPtr);
        Push(sPtr, PolySub(&p, &q));
        PolyDestroy(&p);
        PolyDestroy(&q);
        return false;
    }
    return true;
}

/**
 * Returns the degree of a polynomial in all variables.
 * @param s
 * @return 
 */
bool Deg(Stack *s) {
    if (!Empty(s)) {
        Poly p = Top(s);
        printf("%d\n", PolyDeg(&p));
        return false;
    }
    return true;
}

/**
 * Returns the degree of a polynomial in a variable given by its index.
 * @param s
 * @param idx
 * @return 
 */
bool DegBy(Stack *s, poly_exp_t idx) {
    if (!Empty(s)) {
        Poly p = Top(s);
        printf("%d\n", PolyDegBy(&p, idx));
        return false;
    }
    return true;
}

/**
 * Exaluates the value of a polynomial in the given point.
 * @param sPtr
 * @param x
 * @return 
 */
bool At(Stack **sPtr, poly_coeff_t x) {
    if (!Empty(*sPtr)) {
        Poly p = Pop(sPtr);
        Push(sPtr, PolyAt(&p, x));
        PolyDestroy(&p);
        return false;
    }
    return true;
}

/**
 * Pops a polynomial from the top of the stack.
 * @param sPtr
 * @return 
 */
bool PopPoly(Stack **sPtr) {
    if (!Empty(*sPtr)) {
        Poly p = Pop(sPtr);
        PolyDestroy(&p);
        return false;
    }
    return true;
}

/**
 * Performs a PolyCompose function on the polynomials from the stack.
 * @param sPtr
 * @param count
 * @return 
 */
bool Compose(Stack **sPtr, unsigned count) {
    bool wynik = true;
    Poly p;
    if (!Empty(*sPtr)) {
        p = Pop(sPtr);
    }
    else {
        return wynik;
    }
    Poly x[count];
    unsigned i = 0;
    while (i < count && !Empty(*sPtr)) {
        x[i] = Pop(sPtr);
        i++;
    }
    if (i == count) {
        Push(sPtr, PolyCompose(&p, count, x));
        wynik = false;
    }
    PolyDestroy(&p);
    for (unsigned j = 0; j < i; j++) {
        Poly temp = x[j];
        PolyDestroy(&temp);
    }
    return wynik;
}

/**
 * Recursively prints the argument polynomial.
 * @param p
 */
void PrintHelper(Poly p) {
    if (PolyIsCoeff(&p)) {
        printf("%ld", p.coeff);
    }
    else {
        for (unsigned i = 0; i < p.size; ++i) {
            printf("%c", POLY_OPENING_SEPARATOR);
            PrintHelper(p.arr[i].p);
            printf("%c", POLY_COEFF_EXP_SEPARATOR);
            printf("%d",p.arr[i].exp);
            printf("%c", POLY_CLOSING_SEPARATOR);
            
            if (i < p.size - 1) {
                printf("+");
            }
        }
    }
}

/**
 * Prints the polynomial on the top of the stack.
 * @param s
 * @return 
 */
bool Print(Stack *s) {
    if (Empty(s)) {
        return true;
    }
    PrintHelper(Top(s));
    printf("\n");
    return false;
}

/**
 * Checks the contents of the line.
 * @return 
 */
int CheckLinePolyOrCommand() {
    int result;
    int c = getchar();
    
    if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') {
        result = VAL_IF_COMMAND;
    }
    else if (c == EOF) {
        result = VAL_IF_EOF;
    }
    else {
        result = VAL_IF_POLY;
    }
    ungetc(c, stdin);
    return result;
}

/**
 * Moves buffer to a new line.
 * @return 
 */
int ForwardToNewLine() {
    int c = getchar();
    while (c != EOF && c != '\n') {
        c = getchar();
    }
    ungetc(c, stdin);
}

/**
 * Executes the logic of the program.
 * @return 
 */
int main() {
    CommandAndParam cap;
    int line = CheckLinePolyOrCommand(), lineCount = 1;
    int c = 0;
    bool underflows = false;
    Stack *sPtr;
    Init(&sPtr);
    
    while (c != EOF && line != VAL_IF_EOF) {
        if (line == VAL_IF_POLY) {
            Poly p;
            if (!ParsePoly(&p, lineCount)) {
                Push(&sPtr, p);
            }
        }
        else {
            if (!ParseCommand(&cap, lineCount)) {
                switch (cap.id) {
                    case ZERO_ID: PushZero(&sPtr); break; 
                    case IS_COEFF_ID: underflows = IsCoeff(sPtr); break;
                    case IS_ZERO_ID: underflows = IsZero(sPtr); break;
                    case CLONE_ID: underflows = Clone(&sPtr); break;
                    case ADD_ID: underflows = Add(&sPtr); break;
                    case MUL_ID: underflows = Mul(&sPtr); break;
                    case NEG_ID: underflows = Neg(&sPtr); break;
                    case SUB_ID: underflows = Sub(&sPtr); break;
                    case IS_EQ_ID: underflows = IsEq(&sPtr); break;
                    case DEG_ID: underflows = Deg(sPtr); break;
                    case DEG_BY_ID:
                         underflows = DegBy(sPtr, cap.degByParam); break;
                    case AT_ID: underflows = At(&sPtr, cap.atParam); break;
                    case PRINT_ID: underflows = Print(sPtr); break;
                    case POP_ID: underflows = PopPoly(&sPtr); break;
                    case COMPOSE_ID:
                         underflows = Compose(&sPtr, cap.composeParam); break;
                    default: WrongCommandErrorMsg(lineCount); break;
                }
                if (underflows) {
                    UnderflowErrorMsg(lineCount);
                }
            }
        }
        ForwardToNewLine();
        c = getchar();
        if (c != EOF) {
            line = CheckLinePolyOrCommand();
            lineCount++;
        }
    }
    Clear(&sPtr);
    return 0;
}
