/** @file
   Interface of the polynomials class

   @author Jakub Pawlewicz <pan@mimuw.edu.pl>
   @author Jan Łanecki <jl385826@students.mimuw.edu.pl>
   @copyright University of Warsaw, Poland
   @date 2017-04-24, 2017-05-11, 2017-06-18
*/

#ifndef __POLY_H__
#define __POLY_H__

#include <stdbool.h>
#include <stddef.h>

/** Type of coefficients of polynomials*/
typedef long poly_coeff_t;

/** Type of exponentials of polynomials */
typedef int poly_exp_t;

typedef struct Mono Mono;

/**
 * Structure containing a polynomial
 */
typedef struct Poly {
    poly_coeff_t coeff; ///< coefficient of a constant polynomial
    Mono* list; ///< list of monomials
} Poly;

/**
  * Structure containing a monomial
  * Monomial is `p * x^e`.
  * Coefficient `p` can ba a polynomial.
  * It is then a polynomial in another indeterminate (not in x).
  */
typedef struct Mono {
    Poly p; ///< coefficient
    poly_exp_t exp; ///< exponent
    Mono* next; ///< next term of polynomial
} Mono;

/**
 * Builds a constant polynomial.
 * @param[in] c : coefficient
 * @return polynomial
 */
static inline Poly PolyFromCoeff(poly_coeff_t c)
{
	return (Poly) {.coeff = c, .list = NULL};
}

/**
 * Builds a polynomial that equals zero.
 * @return polynomial
 */
static inline Poly PolyZero()
{
	return PolyFromCoeff(0);
}

/**
 * Builds monomial `p * x^e`.
 * Takes ownership of the polynomial @p p.
 * @param[in] p : polynomial - coefficient of the monomial
 * @param[in] e : exponential
 * @return monomial `p * x^e`
 */
static inline Mono MonoFromPoly(const Poly *p, poly_exp_t e)
{
    return (Mono) {.p = *p, .exp = e, .next = NULL};
}

/**
 * Checks if a polynomial is constant.
 * @param[in] p : polynomial
 * @return
 */
static inline bool PolyIsCoeff(const Poly *p)
{
    return p->list == NULL;
}

/**
 * Checks if a polynomial equals zero.
 * @param[in] p : polynomial
 * @return
 */
static inline bool PolyIsZero(const Poly *p)
{
    return PolyIsCoeff(p) && p->coeff == 0;
}

/**
 * Deletes a polynomial.
 * @param[in] p : polynomial
 */
void PolyDestroy(Poly *p);

/**
 * Deletes a monomial.
 * @param[in] m : monomial
 */
static inline void MonoDestroy(Mono *m)
{
	PolyDestroy(&m->p);
}

/**
 * Makes a deep copy of a polynomial.
 * @param[in] p : polynomial
 * @return copy
 */
Poly PolyClone(const Poly *p);

/**
 * Makes a deep copy of a monomial.
 * @param[in] m : monomial
 * @return copy
 */
static inline Mono MonoClone(const Mono *m)
{
	return (Mono) {.p = PolyClone(&m->p), .exp = m->exp, .next = NULL};
}

/**
 * Adds two polynomials.
 * @param[in] p : polynomial
 * @param[in] q : polynomial
 * @return `p + q`
 */
Poly PolyAdd(const Poly *p, const Poly *q);

/**
 * Sums a list of monomials and builds a polynomial.
 * Takes ownership of the monomials in the @p monos array.
 * @param[in] count : number of monomials
 * @param[in] monos : array of monomials
 * @return polynomial that is a sum of the monomials
 */
Poly PolyAddMonos(unsigned count, const Mono monos[]);

/**
 * Multiplies two polynomials.
 * @param[in] p : polynomial
 * @param[in] q : polynomial
 * @return `p * q`
 */
Poly PolyMul(const Poly *p, const Poly *q);

/**
 * Returns the negation of the polynomial.
 * @param[in] p : polynomial
 * @return `-p`
 */
Poly PolyNeg(const Poly *p);

/**
 * Subtracts the polynomial q from the polynomial p.
 * @param[in] p : polynomial
 * @param[in] q : polynomial
 * @return `p - q`
 */
Poly PolySub(const Poly *p, const Poly *q);

/**
 * Returns the degree of the polynomial in a given variable (return -1 
 * if the polynomial equals 0).
 * Variables indexes start from 0.
 * Varaible with index equal 0 is the main variable in the polynomial.
 * Larger indexes mean variables of the polunomials in the coefficients.
 * @param[in] p : polynomial
 * @param[in] var_idx : variable index
 * @return degree of the polynomial @p p in a variable with @p var_idx index
 */
poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx);

/**
 * Returns the degree of the polynomial @p p (-1 if the polynomial equals 0).
 * @param[in] p : polynomial
 * @return degree of the polynomial @p p
 */
poly_exp_t PolyDeg(const Poly *p);

/**
 * Checks if the two polynomials are equal.
 * @param[in] p : polynomial
 * @param[in] q : polynomial
 * @return `p = q`
 */
bool PolyIsEq(const Poly *p, const Poly *q);

/**
 * Evaluates the value of the polynomial in the point @p x.
 * Substitutes the first variable with the value @p x.
 * @param[in] p
 * @param[in] x
 * @return @f$p(x, x_0, x_1, \ldots)@f$
 */
Poly PolyAt(const Poly *p, poly_coeff_t x);

/**
 * Returns polynomial @p p with every variable @p x_i substitued with polynomial
 * @p x[i] and variables with indexes equal to or larger than count - with 0.
 * @param p
 * @param count
 * @param x
 * @return
 */
Poly PolyCompose(const Poly *p, unsigned count, const Poly x[]);

#endif /* __POLY_H__ */
