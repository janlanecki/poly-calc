/** @file
   Unit tests for PolyCompose function and parser

   @author Jan Łanecki <jl385826@students.mimuw.edu.pl>
   @copyright University of Warsaw, Poland
   @date 2017-06-21
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>
#include "cmocka.h"
#include "poly.h"

#define BUFFER_SIZE 256 ///< size of buffers

extern int calculator_main(); ///< macro for the main function in calc_poly.c

static char fprintf_buffer[BUFFER_SIZE]; ///< stderr buffer
static char printf_buffer[BUFFER_SIZE]; ///< stdout buffer
static int fprintf_position = 0; ///< position in stderr buffer
static int printf_position = 0; ///< position in stdout buffer

/**
 * Mock fprintf function which checks the correctness of stderr messages.
 * @param file
 * @param format
 * @param ...
 * @return 
 */
int mock_fprintf(FILE* const file, const char *format, ...) {
    int return_value;
    va_list args;

    assert_true(file == stderr);
    /* Following assertion checks if the frpintf_position isn't less than 0.
    The buffer must fit in the last byte with the value 0. */
    assert_true((size_t)fprintf_position < sizeof(fprintf_buffer));

    va_start(args, format);
    return_value = vsnprintf(fprintf_buffer + fprintf_position,
                             sizeof(fprintf_buffer) - fprintf_position,
                             format,
                             args);
    va_end(args);

    fprintf_position += return_value;
    assert_true((size_t)fprintf_position < sizeof(fprintf_buffer));
    return return_value;
}

/**
 * Mock printf function which checks the correctness of stdout messages.
 * @param format
 * @param ...
 * @return 
 */
int mock_printf(const char *format, ...) {
    int return_value;
    va_list args;

    /* Following assertion checks if printf_position isn't less than 0.
    Buffer must fit in the last byte with the value 0. */
    assert_true((size_t)printf_position < sizeof(printf_buffer));

    va_start(args, format);
    return_value = vsnprintf(printf_buffer + printf_position,
                             sizeof(printf_buffer) - printf_position,
                             format,
                             args);
    va_end(args);

    printf_position += return_value;
    assert_true((size_t)printf_position < sizeof(printf_buffer));
    return return_value;
}

static char input_stream_buffer[BUFFER_SIZE]; ///< stdin buffer
static int input_stream_position = 0; ///< first position in stdin buffer
static int input_stream_end = 0; ///< last position in stdin buffer
int read_char_count; ///< number of chars in stdin buffer

/**
 * Mock getchar function to intercept input from stdin.
 * @return 
 */
int mock_getchar() {
    if (input_stream_position < input_stream_end)
        return input_stream_buffer[input_stream_position++];
    else
        return EOF;
}

/**
 * Mock ungetc function, only for stdin.
 * @param c
 * @param stream
 * @return 
 */
int mock_ungetc(int c, FILE *stream) {
    assert_true(stream == stdin);
    if (input_stream_position > 0)
        return input_stream_buffer[--input_stream_position] = c;
    else
        return EOF;
}

/**
 * Initializes input for a program using stdin.
 * @param str
 */
static void init_input_stream(const char *str) {
    memset(input_stream_buffer, 0, sizeof(input_stream_buffer));
    input_stream_position = 0;
    input_stream_end = strlen(str);
    assert_true((size_t)input_stream_end < sizeof(input_stream_buffer));
    strcpy(input_stream_buffer, str);
}

/**
 * Tests PolyCompose with a polynomial equal 0 and the count argument equal 0.
 * @param state
 */
static void test_poly_zero_count_zero(void **state) {
    (void) state;
    Poly zero = PolyZero();
    Poly arr[1] = { PolyZero() };
    Poly composed = PolyCompose(&zero, 0, arr);
    
    assert(PolyIsZero(&composed));
    
    PolyDestroy(arr);
    PolyDestroy(&zero);
    PolyDestroy(&composed);
}

/**
 * Tests PolyCompose with a polynomial equal 0 and the count argument equal 1.
 * @param state
 */
static void test_poly_zero_count_one(void **state) {
    (void) state;
    Poly zero_1 = PolyZero();
    Poly zero_2 = PolyZero();
    Poly arr[1] = { zero_2 };
    Poly composed = PolyCompose(&zero_1, 1, arr);
    
    assert(PolyIsZero(&composed));
    
    PolyDestroy(&zero_1);
    PolyDestroy(&zero_2);
    PolyDestroy(&composed);
}

/**
 * Tests PolyCompose with a constant polynomial and the count argument equal 0.
 * @param state
 */
static void test_poly_const_count_zero(void **state) {
    (void) state;
    poly_coeff_t coeff = 42;
    Poly arr[1] = { PolyZero() };
    Poly poly_coeff = PolyFromCoeff(coeff);
    Poly composed = PolyCompose(&poly_coeff, 0, arr);
    
    assert(PolyIsEqual(&composed, &poly_coeff));
    
    PolyDestroy(arr);
    PolyDestroy(&poly_coeff);
    PolyDestroy(&composed);
}

/**
 * Tests PolyCompose with a constant polynomial and the count argument equal 1.
 * @param state
 */
static void test_poly_const_count_one(void **state) {
    (void) state;
    poly_coeff_t coeff = 42;
    Poly poly_coeff_1 = PolyFromCoeff(coeff);
    Poly poly_coeff_2 = PolyFromCoeff(coeff + 1);
    Poly arr[1] = { poly_coeff_2 };
    Poly composed = PolyCompose(&poly_coeff_1, 1, arr);
    
    assert(PolyIsEqual(&composed, &poly_coeff_1));
    
    PolyDestroy(&poly_coeff_1);
    PolyDestroy(&poly_coeff_2);
    PolyDestroy(&composed);    
}

/** 
 * Tests PolyCompose with a univariate polynomial and the count argument equal 0.
 * @param state
 */
static void test_poly_x_count_zero(void **state) {
    (void) state;
    Poly poly_coeff = PolyFromCoeff(1);
    Poly arr[1] = { PolyZero() };
    Mono m = MonoFromPoly(&poly_coeff, 1);
    Poly p = PolyAddMonos(1, &m);
    Poly composed = PolyCompose(&p, 0, arr);
    
    assert(PolyIsZero(&composed));
    
    PolyDestroy(&p);
    PolyDestroy(&composed);
}

/**
 * Tests PolyCompose with a univariate polynomial, the count argument equal 1 
 * and a constant polynomial argument.
 * @param state
 */
static void test_poly_x_count_one_const(void **state) {
    (void) state;
    Poly poly_coeff = PolyFromCoeff(1);
    Poly arr[1] = { PolyZero() };
    Mono m = MonoFromPoly(&poly_coeff, 1);
    Poly p = PolyAddMonos(1, &m);
    Poly composed = PolyCompose(&p, 1, arr);
    
    assert(PolyIsZero(&composed));
    
    PolyDestroy(&poly_coeff);
    PolyDestroy(&p);
    PolyDestroy(&composed);
}

/**
 * Tests PolyCompose with a univariate polynomial, the count argument equal 1 
 * and a univariate polynomial argument.
 * @param state
 */
static void test_poly_x_count_one_x(void **state) {
    (void) state;
    Poly poly_coeff = PolyFromCoeff(1);
    Mono m = MonoFromPoly(&poly_coeff, 1);
    Poly p = PolyAddMonos(1, &m);
    Poly arr[1] = { p };
    Poly composed = PolyCompose(&p, 1, arr);
    
    assert(PolyIsEqual(&composed, &p));
    
    PolyDestroy(&poly_coeff);
    PolyDestroy(&p);
    PolyDestroy(&composed);
}

/**
 * Tests PolyCompose parser with no parameter.
 * @param state
 */
static void test_parse_no_parameter(void **state) {
    (void) state;
    init_input_stream("COMPOSE ");
    calculator_main();
    
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
    assert_string_equal(printf_buffer, "");
}

/**
 * Tests PolyCompose parser with a parameter equal 0.
 * @param state
 */
static void test_parse_zero(void **state) {
    (void) state;
    init_input_stream("COMPOSE 0");
    calculator_main();
    
    assert_string_equal(fprintf_buffer, "ERROR 1 STACK UNDERFLOW\n");
    assert_string_equal(printf_buffer, "");
}

/**
 * Tests PolyCompose parser with a maximum parameter value.
 * @param state
 */
static void test_parse_max(void **state) {
    (void) state;
    init_input_stream("COMPOSE 4294967295");
    calculator_main();
    
    assert_string_equal(fprintf_buffer, "ERROR 1 STACK UNDERFLOW\n");
    assert_string_equal(printf_buffer, "");
}

/**
 * Tests PolyCompose parser with a minimum parameter value - 1.
 * @param state
 */
static void test_parse_min_minus_one(void **state) {
    (void) state;
    init_input_stream("COMPOSE -1");
    calculator_main();
    
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
    assert_string_equal(printf_buffer, "");
}

/**
 * Tests PolyCompose parser with a maximum parameter value plus 1.
 * @param state
 */
static void test_parse_max_plus_one(void **state) {
    (void) state;
    init_input_stream("COMPOSE 4294967296");
    calculator_main();
    
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");   
    assert_string_equal(printf_buffer, "");
}

/**
 * Tests PolyCompose parser with a very large number.
 * @param state
 */
static void test_parse_much_more_than_max(void **state) {
    (void) state;
    init_input_stream("COMPOSE 424242424242424242424242424242");
    calculator_main();
    
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
    assert_string_equal(printf_buffer, "");
}

/**
 * Tests PolyCompose parser with a parameter which consists of letters.
 * @param state
 */
static void test_parse_letters(void **state) {
    (void) state;
    init_input_stream("COMPOSE aAbBcCdDeE");
    calculator_main();
    
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n"); 
    assert_string_equal(printf_buffer, "");
}

/**
 * Tests PolyCompose parser with parameter which consists of letters and digits.
 * @param state
 */
static void test_parse_digits_and_letters(void **state) {
    (void) state;
    init_input_stream("COMPOSE 123aAaA123C");
    calculator_main();
    
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
    assert_string_equal(printf_buffer, "");
}

/** Initializes the context of the tests. */
static int test_setup(void **state) {
    memset(fprintf_buffer, 0, sizeof(fprintf_buffer));
    memset(printf_buffer, 0, sizeof(printf_buffer));
    printf_position = 0;
    fprintf_position = 0;

    return 0;
}

/**
 * Executes the tests.
 * @return 
 */
int main() {
    const struct CMUnitTest compose_calculations_tests[] = {
        cmocka_unit_test(test_poly_zero_count_zero),
        cmocka_unit_test(test_poly_zero_count_one),
        cmocka_unit_test(test_poly_const_count_zero),
        cmocka_unit_test(test_poly_const_count_one),
        cmocka_unit_test(test_poly_x_count_zero),
        cmocka_unit_test(test_poly_x_count_one_const),
        cmocka_unit_test(test_poly_x_count_one_x)
    };
    
    const struct CMUnitTest compose_parser_tests[] = {
        cmocka_unit_test_setup(test_parse_no_parameter, test_setup),
        cmocka_unit_test_setup(test_parse_zero, test_setup),
        cmocka_unit_test_setup(test_parse_max, test_setup),
        cmocka_unit_test_setup(test_parse_min_minus_one, test_setup),
        cmocka_unit_test_setup(test_parse_max_plus_one, test_setup),
        cmocka_unit_test_setup(test_parse_much_more_than_max, test_setup),
        cmocka_unit_test_setup(test_parse_letters, test_setup),
        cmocka_unit_test_setup(test_parse_digits_and_letters, test_setup),
    };
    
    int res;
    res = cmocka_run_group_tests(compose_calculations_tests, NULL, NULL);
    res += cmocka_run_group_tests(compose_parser_tests, NULL, NULL);
    return res;
}
