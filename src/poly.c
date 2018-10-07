/** @file
   Implementation of the polynomial class

   @author Jan Łanecki <jl385826@students.mimuw.edu.pl>
   @copyright University of Warsaw, Poland
   @date 2017-05-11, 2017-06-18
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "poly.h"

void PolyDestroy(Poly *p) {
    if (PolyIsCoeff(p))
        return;
    Mono* m = p->list;
    while (m != NULL) {
        Mono* next = m->next;
        MonoDestroy(m);
        free(m);
        m = next;
    }
    p->list = NULL;
}

Poly PolyClone(const Poly *p) {
    if (PolyIsCoeff(p))
        return *p;
    Poly r;
    Mono *mp = p->list, *mr = malloc(sizeof(Mono));
    assert(mr != NULL);
    r.list = mr;
    while (mp->next != NULL) {
        *mr = MonoClone(mp);
        mr->next = malloc(sizeof(Mono));
        assert(mr->next != NULL);
        mp = mp->next;
        mr = mr->next;
    }
    *mr = MonoClone(mp);
    mr->next = NULL;
    return r;
}

static Poly PolyAddCoeff(const Poly* p, poly_coeff_t c );

/**
 * Adds a coefficient to a polynomial which isn't a constant.
 * @param[in] p : non constant polynomial
 * @param[in] c : coefficient
 * @return
 */
static Poly PolyAddPolyCoeff(const Poly* p, poly_coeff_t c) {
    assert(!PolyIsCoeff(p));
    if (c == 0)
        return PolyClone(p);
    Poly r;
    Mono *mp = p->list, *mr;
    if (mp->exp == 0) {
        Poly temp = PolyAddCoeff(&p->list->p, c);
        if (PolyIsZero(&temp)) {
            assert(mp->next != NULL);
            mp = mp->next;
            mr = malloc(sizeof(Mono));
            assert(mr != NULL);
            r.list = mr;
            *mr = MonoClone(mp);
            mp = mp->next;
        }
        else if (PolyIsCoeff(&temp) && mp->next == NULL) {
            return temp;
        }
        else {
            mr = malloc(sizeof(Mono));
            assert(mr != NULL);
            r.list = mr;
            *mr = MonoFromPoly(&temp, 0);
            mp = mp->next;
        }
    }
    else {
        mr = malloc(sizeof(Mono));
        assert(mr != NULL);
        r.list = mr;
        *mr = (Mono) {.p = PolyFromCoeff(c), .exp = 0, .next = NULL};
    }

    while(mp != NULL) {
        mr->next = malloc(sizeof(Mono));
        assert(mr->next != NULL);
        mr = mr->next;
        *mr = MonoClone(mp);
        mp = mp->next;
    }
    mr->next = NULL;
    return r;
}

/**
 * Adds a coefficient to a polynomial.
 * @param[in] p : polynomial
 * @param[in] c : coefficient
 * @return `p + c`
 */
static Poly PolyAddCoeff(const Poly* p, poly_coeff_t c) {
    if (PolyIsCoeff(p))
        return PolyFromCoeff(p->coeff + c);
    else
        return PolyAddPolyCoeff(p, c);
}

/**
 * Adds two normal polynomials.
 * @param[in] p : non constant polynomial
 * @param[in] q : non constant polynomial
 * @return
 */
static Poly PolyAddPolyPoly(const Poly *p, const Poly *q) {
    assert(!PolyIsCoeff(p) && !PolyIsCoeff(q));
    bool added = true;
    Mono *mp = p->list, *mq = q->list, *mr = malloc(sizeof(Mono)), *mprev = NULL;
    assert(mr != NULL);
    Poly r = (Poly) {.list = mr};
    while (mp != NULL || mq != NULL) {
        if (mq == NULL || mp != NULL && mp->exp < mq->exp) {
            assert(!PolyIsZero(&mp->p));
            *mr = MonoClone(mp);
            mp = mp->next;
        }
        else if (mp == NULL || mp->exp > mq->exp) {
            assert(!PolyIsZero(&mq->p));
            *mr = MonoClone(mq);
            mq = mq->next;
        }
        else {
            Poly temp = PolyAdd(&mp->p, &mq->p);
            if (!PolyIsZero(&temp))
                *mr = MonoFromPoly(&temp, mp->exp);
            else
                added = false;
            mp = mp->next;
            mq = mq->next;
        }

        if (added) {
            mr->next = malloc(sizeof(Mono));
            assert(mr->next != NULL);
            mprev = mr;
            mr = mr->next;
        }
        else
            added = true;
    }
    if (mr != r.list) {
        free(mr);
        mprev->next = NULL;
        if (r.list->next == NULL && r.list->exp == 0
            && PolyIsCoeff(&r.list->p)) {
            Poly temp = PolyFromCoeff(r.list->p.coeff);
            free(r.list);
            r = temp;
        }
    }
    else { //nothing was added
        r = PolyFromCoeff(0);
        free(mr);
    }
    return r;
}

Poly PolyAdd(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p))
        return PolyAddCoeff(q, p->coeff);
    else if (PolyIsCoeff(q))
        return PolyAddCoeff(p, q->coeff);
    else
        return PolyAddPolyPoly(p, q);
}

/**
 * Compares monomials.
 * @param a : monomial pointer
 * @param b : monomial pointer
 * @return -1 <, 0 =, 1 >
 */
static poly_exp_t MonoCmp(const void* m, const void* n) {
    return ((const Mono *) m)->exp - ((const Mono *) n)->exp;
}

Poly PolyAddMonos(unsigned count, const Mono monos[]) {
    Mono *arr = calloc(count, sizeof(Mono));
    assert(arr != NULL);
    memcpy(arr, monos, count * sizeof(Mono));
    qsort(arr, count, sizeof(Mono), MonoCmp);
    Mono *dummy, *head, *prev = NULL;
    dummy = head = malloc(sizeof(Mono));
    assert(dummy != NULL);
    for (unsigned i = 0; i < count; i++) {
        if (head == dummy || head->exp != arr[i].exp) {
            assert(!PolyIsZero(&arr[i].p));
            head->next = malloc(sizeof(Mono));
            assert(head->next != NULL);
            prev = head;
            head = head->next;
            *head = arr[i];
        }
        else {
            Mono m = (Mono) {.p = PolyAdd(&head->p, &arr[i].p),
                             .exp = head->exp, .next = NULL};
            PolyDestroy(&head->p);
            PolyDestroy(&arr[i].p);
            if (PolyIsZero(&m.p)) {
                free(head);
                head = prev;
                prev->next = NULL;
            }
            else
                *head = m;
        }
    }
    Poly r = (Poly) {.list = dummy->next};
    free(dummy);
    if (r.list == NULL)
        r.coeff = 0;
    else if (r.list->next == NULL && r.list->exp == 0
             && PolyIsCoeff(&r.list->p)) {
        Poly temp = r.list->p;
        free(r.list);
        r = temp;
    }
    free(arr);
    return r;
}
/**
 * Returns number of terms of a polynomial.
 * @param p
 * @return
 */
static unsigned PolyLength(const Poly* p) {
    if (PolyIsCoeff(p))
        return 0;
    else {
        unsigned count = 0;
        Mono* m = p->list;
        while (m != NULL) {
            count++;
            m = m->next;
        }
        return count;
    }
}

/**
 * Muliplies two non constant polynomials.
 * @param[in] p : non constant polynomial
 * @param[in] q : non constant polynomial
 * @return `p * q`
 */
static Poly PolyMulPolyPoly(const Poly *p, const Poly *q) {
    unsigned p_len = PolyLength(p), q_len = PolyLength(q);
    unsigned count = p_len * q_len, k = 0;
    Mono* arr = calloc(count, sizeof(struct Mono));
    Mono *mp = p->list, *mq = q->list;
    for (unsigned i = 0; i < p_len; i++) {
        for (unsigned j = 0; j < q_len; j++) {
            arr[k++] = (Mono) {.p = PolyMul(&mp->p, &mq->p),
                               .exp = mp->exp + mq->exp, .next = NULL};
            mq = mq->next;
        }
        mq = q->list;
        mp = mp->next;
    }
    Poly r = PolyAddMonos(count, arr);
    free(arr);
    return r;
}

static Poly PolyMulCoeff(const Poly *p, poly_coeff_t c);

/**
 * Muliplies a non constant polynomial by a coefficient.
 * @param[in] p : non constant polynomial
 * @param[in] c : coefficient
 * @return `p * c`
 */
static Poly PolyMulPolyCoeff(const Poly *p, poly_coeff_t c) {
    if (c == 0)
        return PolyZero();
    Mono *mr = malloc(sizeof(Mono)), *mp = p->list;
    assert(mr != NULL);
    Mono *mprev = mr;
    Poly r = (Poly) {.list = mr};
    while (mp != NULL) {
        Mono m = (Mono) {.p = PolyMulCoeff(&mp->p, c), .exp = mp->exp,
                         .next = NULL};
        if (!PolyIsZero(&m.p)) {
            *mr = m;
            mprev = mr;
            mr->next = malloc(sizeof(Mono));
            assert(mr->next != NULL);
            mr = mr->next;
        }
        mp = mp->next;
    }
    if (mprev == mr)
        r = PolyZero();
    else
        mprev->next = NULL;
    free(mr);
    return r;
}

/**
 * Multiplies a polynomial by a coefficient.
 * @param[in] p : polynomial
 * @param[in] c : coefficient
 * @return `p * c`
 */
static Poly PolyMulCoeff(const Poly *p, poly_coeff_t c) {
    if (PolyIsCoeff(p))
        return PolyFromCoeff(p->coeff * c);
    else
        return PolyMulPolyCoeff(p, c);
}

Poly PolyMul(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p))
        return PolyMulCoeff(q, p->coeff);
    else if (PolyIsCoeff(q))
        return PolyMulCoeff(p, q->coeff);
    else
        return PolyMulPolyPoly(p, q);
}

Poly PolyNeg(const Poly *p) {
    if (PolyIsCoeff(p))
        return PolyFromCoeff(-1 * p->coeff);
    Mono *mn = malloc(sizeof(Mono)), *mp = p->list;
    assert(mn != NULL);
    Poly neg = (Poly) {.list = mn};
    while(mp != NULL) {
        mn->p = PolyNeg(&mp->p);
        mn->exp = mp->exp;
        if (mp->next == NULL)
            mn->next = NULL;
        else {
            mn->next = malloc(sizeof(Mono));
            assert(mn->next != NULL);
            mn = mn->next;
        }
        mp = mp->next;
    }
    return neg;
}

Poly PolySub(const Poly *p, const Poly *q) {
    Poly neg = PolyNeg(q);
    Poly r = PolyAdd(p, &neg);
    PolyDestroy(&neg);
    return r;
}

poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx) {
    if (PolyIsCoeff(p)) {
        if (PolyIsZero(p))
            return -1;
        else
            return 0;
    }
    else if (var_idx == 0) {
        Mono *m = p->list;
        while (m->next != NULL)
            m = m->next;
        return m->exp;
    }
    else {
        poly_exp_t max = -1;
        Mono *m = p->list;
        while (m != NULL) {
            poly_exp_t deg = PolyDegBy(&m->p, var_idx - 1);
            if (deg > max)
                max = deg;
            m = m->next;
        }
        return max;
    }
}

poly_exp_t PolyDeg(const Poly *p) {
    if (PolyIsCoeff(p)) {
        if (PolyIsZero(p))
            return -1;
        else
            return 0;
    }
    else {
        poly_exp_t max = -1;
        Mono *m = p->list;
        while (m != NULL) {
            poly_exp_t deg = PolyDeg(&m->p) + m->exp;
            if (deg > max)
                max = deg;
            m = m->next;
        }
        return max;
    }
}

bool PolyIsEq(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p) != PolyIsCoeff(q))
        return false;
    else if (PolyIsCoeff(p))
        return p->coeff == q->coeff;
    else {
        Mono *mp = p->list, *mq = q->list;
        while (mp != NULL && mq != NULL) {
            if (mp->exp != mq->exp || !PolyIsEq(&mp->p, &mq->p))
                return false;
            mp = mp->next;
            mq = mq->next;
        }
        if (mp == NULL && mq == NULL)
            return true;
        else
            return false;
    }
}

/**
 * Evaluates @f$base^exp@f$.
 * @param[in] base
 * @param[in] exp
 * @return @f$base^exp@f$
 */
static poly_coeff_t BinPower(poly_coeff_t base, poly_exp_t exp) {
    poly_coeff_t result = 1;
    while (exp > 0) {
        if (exp % 2)
            result *= base;
        base *= base;
        exp /= 2;
    }
    return result;
}

Poly PolyAt(const Poly *p, poly_coeff_t x) {
    if (PolyIsCoeff(p))
        return PolyClone(p);
    Poly r = PolyZero();
    poly_exp_t exp = 0;
    poly_coeff_t pow = 1;
    Mono *m = p->list;
    while (m != NULL) {
        pow *= BinPower(x, m->exp - exp);
        exp = m->exp;
        Poly pi = PolyMulCoeff(&m->p, pow);
        Poly q = PolyAdd(&r, &pi);
        PolyDestroy(&pi);
        PolyDestroy(&r);
        r = q;
        m = m->next;
    }
    return r;
}

/**
 * Returns polynomial @p p with each variable @p x_i substitued with
 * polynomial @p x[i]. Variables with index equal to or larger than
 * count are substituted with 0.
 * Takes ownership of the polynomial @p p.
 * @param p
 * @param count
 * @param x
 * @param idx
 * @return
 */
static void PolyComposeHelp(Poly *p, unsigned count, const Poly x[], unsigned idx) {
    if (count == idx) {
        Poly temp = PolyFromCoeff(PolyAtZeros(p));
        PolyDestroy(p);
        *p = temp;
    }
    else if (!PolyIsCoeff(p)) {
        for (int i = 0; i < p->size; i++)
            PolyComposeHelp(&p->arr[i].p, count, x, idx + 1);
        Poly insert = x[idx];
        PolyComposeAtRoot(p, &insert);
    }
}

Poly PolyCompose(const Poly *p, unsigned count, const Poly x[]) {
    Poly clone = PolyClone(p);
    PolyComposeHelp(&clone, count, x, 0);
    return clone;
}
