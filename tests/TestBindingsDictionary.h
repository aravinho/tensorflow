#ifndef TEST_BINDINGSDICTIONARY_H
#define TEST_BINDINGSDICTIONARY_H

#include "stdlib.h"

using namespace std;


/* Tests for the BindingsDictionary class. */

void test_bd_constructor();
void test_bd_destructor();
void test_bd_add_variable();
void test_bd_bind_value();
void test_bd_get_value();
void test_bd_has_been_declared();
void test_bd_has_been_defined();

void run_bd_tests();


#endif