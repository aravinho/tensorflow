#ifndef TEST_INTERPRETER_H
#define TEST_INTERPRETER_H

#include "stdlib.h"

using namespace std;


/* Tests for the Interpreter class. */

void test_interp_constructor_interpret_destructor();
void test_interp_parse_line();
void test_interp_apply_binary_operation();
void test_interp_apply_unary_operation();

void run_interp_tests();


#endif