/** @file
   Interface of the polynomial stack class

   @author Jan Łanecki <jl385826@students.mimuw.edu.pl>
   @copyright University of Warsaw, Poland
   @date 2017-05-27
*/

#ifndef __STACK_POLY_H__
#define __STACK_POLY_H__

#include "poly.h"

/**
 * Structure containing the stack of polynomials
 */
typedef struct stack {
    Poly p; ///< polynomial
    struct stack *next; ///< next stack element
} Stack;

void Init(Stack **s);

void Push(Stack **s, Poly p);

Poly Pop(Stack **s);

Poly Top(Stack *s);

bool Empty(Stack *s);

bool Full(Stack *s);

void Clear(Stack **s);

#endif /* __STACK_POLY_H__ */
