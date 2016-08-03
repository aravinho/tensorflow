#include <stdio.h>
#include <iostream>

#include "TestNode.h"
#include "TestDataFlowGraph.h"
#include "TestBindingsDictionary.h"
#include "TestPreprocessor.h"
#include "TestCompiler.h"
#include "TestInterpreter.h"
#include "TestGradientDescent.h"

using namespace std;

int main(int argc, char *argv[]) {
	run_node_tests();
	run_dfg_tests();
	run_bd_tests();
	run_pp_tests();
	run_comp_tests();
	run_interp_tests();
	run_gd_tests();
	return 0;
}