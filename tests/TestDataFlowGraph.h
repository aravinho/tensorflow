#ifndef TEST_DATAFLOWGRAPH_H
#define TEST_DATAFLOWGRAPH_H

#include "stdlib.h"

using namespace std;


/* Tests for the DataFlowGraph class. */

void test_dfg_constructor();
void test_dfg_destructor();
void test_dfg_add_node();
void test_dfg_get_node();
void test_dfg_add_flow_edge();
void test_dfg_get_num_nodes();
void test_dfg_get_loss_node();
void test_dfg_clear_all_markings();
void test_dfg_top_sort();

void run_dfg_tests();


#endif