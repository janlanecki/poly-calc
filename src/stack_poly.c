/** @file
   Implementation of the polynomial stack class

   @author Jan Łanecki <jl385826@students.mimuw.edu.pl>
   @copyright University of Warsaw, Poland
   @date 2017-05-25
*/

#include <stdlib.h>
#include <assert.h>
#include "stack_poly.h"

bool Empty(Stack *s) {
    return (s == NULL);
}

void Init(Stack **s) {
    *s = NULL;
}

void Push(Stack **s, Poly p) {
    Stack *temp = malloc(sizeof(Stack));
    assert(temp != NULL);
    temp->next = *s;
    temp->p = p;
    *s = temp;
}

Poly Pop(Stack **s) {
    Stack *temp = *s;
    Poly p = (*s)->p;
    *s = (*s)->next;
    free(temp);
    return p;
}

Poly Top(Stack *s) {
    return s->p;
}

bool Full(Stack *s) { 
    Stack *temp = malloc(sizeof(Stack));
    
    if (temp == NULL) {
        return (true);
    }
    else {
        free(temp); 
        return (false);
    }
}

void Clear(Stack **s) {
    while (!Empty(*s)) {
        Poly p = Pop(s);
        PolyDestroy(&p);
    }
}
