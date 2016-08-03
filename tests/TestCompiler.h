#ifndef TEST_COMPILER_H
#define TEST_COMPILER_H

#include "stdlib.h"

using namespace std;


/* Tests for the Compiler class. */

void test_comp_constructor_compile_destructor();
void test_comp_parse_line();
void test_comp_duplicate_line_for_gcp();
void test_comp_generate_partial_var_name();
void test_comp_generate_intvar_name();
void test_comp_declare_partial_lambda();
void test_comp_define_partial_lambda();
void test_comp_declare_child_partials();
void test_comp_define_child_partials();

void run_comp_tests();


#endif