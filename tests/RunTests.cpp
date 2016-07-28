#include <stdio.h>
#include "TestNode.h"
#include "TestDataFlowGraph.h"
#include "TestBindingsDictionary.h"
#include "TestPreprocessor.h"

using namespace std;

int main(int argc, char *argv[]) {
	run_node_tests();
	run_dfg_tests();
	run_bd_tests();
	run_pp_tests();
	return 0;
}