#ifndef TEST_NODE_H
#define TEST_NODE_H

#include "stdlib.h"

using namespace std;


/* Tests for the Node class. */

void test_node_constructor();
void test_node_destructor();
void test_node_name();
void test_node_type();
void test_node_is_constant();
void test_node_operation();
void test_node_parent();
void test_node_children();
void test_node_mark();

void run_node_tests();


#endif