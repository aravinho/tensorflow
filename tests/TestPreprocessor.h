#ifndef TEST_PREPROCESSOR_H
#define TEST_BINDINGSDICTIONARY_H

#include "stdlib.h"

using namespace std;


/* Tests for the Preprocessor class. */

void test_pp_constructor();
void test_pp_destructor();
void test_pp_expand_line();
void test_pp_expand_declare_vector();
void test_pp_expand_define();
void test_pp_expand_vector_operation();
void test_pp_expand_unary_macro();
void test_pp_expand_binary_macro();
void test_pp_substitute_dummy_names();
void test_pp_is_binary_macro();
void test_pp_is_unary_macro();
void test_pp_expand_dot_product();
void test_pp_expand_vector_add();
void test_pp_expand_vector_mul();
void test_pp_expand_vector_scale();
void test_pp_expand_vector_increment();
void test_pp_parse_macro_line();
void test_pp_parse_macro_first_line();
void test_pp_parse_macro_subsequent_line();
void test_pp_is_valid_declare_line();
void test_pp_is_valid_define_line();
void test_pp_is_valid_declare_vector_line();
void test_pp_is_valid_macro();

void run_pp_tests();


#endif