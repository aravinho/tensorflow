#include <iostream>
#include <fstream>

#include "TestCompiler.h"
#include "../Compiler.h"
#include "TestUtilities.h"

using namespace std;

void test_comp_constructor_compile_destructor() {

	// make sure constructor properly initializes the DFG
	Compiler *c = new Compiler();
	assert_equal_int(c->get_dfg()->get_num_nodes(), 0, "test_comp_constructor_compile_destructor");

	// make sure the test Expanded Shape Program is correctly compiled into the GCP
	assert_equal_int(c->compile("tests/test_files/inputs/expanded_shape_complex.tf", "tests/test_files/outputs/gcp_complex.tf"), 0, "test_comp_constructor_compile_destructor");
	assert_identical_files("tests/test_files/outputs/gcp_complex.tf", "tests/test_files/exp_outputs/gcp_complex.tf", "test_comp_constructor_compile_destructor");

	// make sure the destructor deletes the DFG without problems
	delete c;

	pass("test_comp_constructor_compile_destructor");

}

void test_comp_compile() {

	cout << "TEST COMPILE NOT YET IMPLEMENTED" << endl;
}

void test_comp_parse_line() {

	Compiler c;

	// simple errors
	assert_equal_int(c.parse_line(""), 0, "test_comp_parse_line");
	assert_equal_int(c.parse_line("declare x"), INVALID_LINE, "test_comp_parse_line");
	assert_equal_int(c.parse_line("definitive x = 3"), INVALID_LINE, "test_comp_parse_line");
	
	// some basic error checking for declare lines
	assert_equal_int(c.parse_line("declare input x x"), INVALID_LINE, "test_comp_parse_line");
	assert_equal_int(c.parse_line("declare expoutput y"), BAD_VAR_TYPE, "test_comp_parse_line");
	assert_equal_int(c.parse_line("declare intvar add"), INVALID_VAR_NAME, "test_comp_parse_line");

	// successful declaration of an input variable
	// make sure an input node named X was successfully added to the DFG
	assert_equal_int(c.parse_line("declare input x"), 0, "test_pp_comp_parse_line");
	assert_equal_int(c.get_dfg()->get_num_nodes(), 1, "test_pp_comp_parse_line");
	assert_true(c.get_dfg()->get_node("x")->get_type() == VariableType::INPUT, "New node X should be an INPUT node", "test_comp_parse_line");

	// some basic error checking for declare lines
	assert_equal_int(c.parse_line("define x 3"), INVALID_LINE, "test_comp_parse_line");
	assert_equal_int(c.parse_line("define mul = add x x"), INVALID_VAR_NAME, "test_comp_parse_line");
	assert_equal_int(c.parse_line("define z = add x x"), INVALID_LINE, "test_comp_parse_line");

	// declaration and successful definition of intvar z as a constant
	// make sure an intvar Z node with two constant children was added to the DFG
	assert_equal_int(c.parse_line("declare intvar z"), 0, "test_pp_comp_parse_line");
	assert_equal_int(c.parse_line("define z = -2.3"), 0, "test_pp_comp_parse_line");

	assert_equal_int(c.get_dfg()->get_num_nodes(), 2, "test_pp_comp_parse_line");
	assert_true(c.get_dfg()->get_node("z")->get_type() == VariableType::INTVAR, "New node Z should be an INTVAR node", "test_comp_parse_line");
	assert_true(c.get_dfg()->get_node("z")->get_operation() == OperationType::ADD, "New node Z should have an ADD operation", "test_comp_parse_line");
	assert_true(c.get_dfg()->get_node("z")->get_child_one()->is_constant(), "Z's children should be constant", "test_comp_parse_line");
	assert_true(c.get_dfg()->get_node("z")->get_child_two()->is_constant(), "Z's children should be constant", "test_comp_parse_line");
	assert_equal_string(c.get_dfg()->get_node("z")->get_child_one_name(), "-2.3", "test_comp_parse_line");
	assert_equal_string(c.get_dfg()->get_node("z")->get_child_two_name(), "0", "test_comp_parse_line");

	// successful declaration of output P
	assert_equal_int(c.parse_line("declare output p"), 0, "test_pp_comp_parse_line");
	// unsuccessful definition of P as equivalent to a non-existent variable
	assert_equal_int(c.parse_line("define p = non_existent_var"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_comp_parse_line");
	
	// successful definition of output P as equivalent to Z
	// make sure an output P node (add Z 0) is added to the DFG
	assert_equal_int(c.parse_line("define p = z"), 0, "test_pp_comp_parse_line");

	assert_equal_int(c.get_dfg()->get_num_nodes(), 3, "test_pp_comp_parse_line");
	assert_true(c.get_dfg()->get_node("p")->get_type() == VariableType::OUTPUT, "New node P should be an OUTPUT node", "test_comp_parse_line");
	assert_true(c.get_dfg()->get_node("p")->get_operation() == OperationType::ADD, "New node P should have an ADD operation", "test_comp_parse_line");
	assert_true(c.get_dfg()->get_node("p")->get_child_one() == c.get_dfg()->get_node("z"), "P's first child should be Z", "test_comp_parse_line");
	assert_true(c.get_dfg()->get_node("p")->get_child_two()->is_constant(), "P's second child should be constant (0)", "test_comp_parse_line");
	assert_equal_string(c.get_dfg()->get_node("p")->get_child_two_name(), "0", "test_comp_parse_line");


	// successful declaration of intvar Q
	assert_equal_int(c.parse_line("declare intvar q"), 0, "test_pp_comp_parse_line");
	// unsuccessful definition of P as a unary primitive
	assert_equal_int(c.parse_line("define q = ln z p"), INVALID_LINE, "test_pp_comp_parse_line");

	// successful definition of intvar Q as e^P
	// make sure an intvar Q node is added to the DFG, with operation EXP and the correct children
	assert_equal_int(c.parse_line("define q = exp p"), 0, "test_pp_comp_parse_line");

	assert_equal_int(c.get_dfg()->get_num_nodes(), 4, "test_pp_comp_parse_line");
	assert_true(c.get_dfg()->get_node("q")->get_type() == VariableType::INTVAR, "New node Q should be an INTVAR node", "test_comp_parse_line");
	assert_true(c.get_dfg()->get_node("q")->get_operation() == OperationType::EXP, "New node Q should have an EXP operation", "test_comp_parse_line");
	assert_true(c.get_dfg()->get_node("q")->get_child_one() == c.get_dfg()->get_node("p"), "Q's first child should be P", "test_comp_parse_line");
	assert_equal_int(c.get_dfg()->get_node("q")->get_num_children(), 1, "test_comp_parse_line");


	// successful declaration of loss L
	assert_equal_int(c.parse_line("declare loss l"), 0, "test_pp_comp_parse_line");
	// unsuccessful definition of L as a binary primitive
	assert_equal_int(c.parse_line("define l = pow q"), INVALID_LINE, "test_pp_comp_parse_line");

	// successful definition of loss L as mul 3.9 Q
	// make sure an loss L node is added to the DFG, with operation MUL and the correct children
	assert_equal_int(c.parse_line("define l = mul 3.9 q"), 0, "test_pp_comp_parse_line");

	assert_equal_int(c.get_dfg()->get_num_nodes(), 5, "test_pp_comp_parse_line");
	assert_true(c.get_dfg()->get_node("l")->get_type() == VariableType::LOSS, "New node L should be an LOSS node", "test_comp_parse_line");
	assert_true(c.get_dfg()->get_node("l")->get_operation() == OperationType::MUL, "New node L should have a MUL operation", "test_comp_parse_line");
	assert_true(c.get_dfg()->get_node("l")->get_child_one()->is_constant(), "L's first child should be constant (3.9)", "test_comp_parse_line");
	assert_equal_string(c.get_dfg()->get_node("l")->get_child_one_name(), "3.9", "test_comp_parse_line");
	assert_true(c.get_dfg()->get_node("l")->get_child_two() == c.get_dfg()->get_node("q"), "L's second child should be Q", "test_comp_parse_line");

	pass("test_comp_parse_line");

}

void test_comp_duplicate_line_for_gcp() {

	Compiler c;
	ofstream write_scratch_file;

	// simple errors
	assert_equal_int(c.duplicate_line_for_gcp("declare input x", write_scratch_file), OTHER_ERROR, "test_comp_duplicate_line_for_gcp");
	write_scratch_file.open("scratch.tf");
	assert_equal_int(c.duplicate_line_for_gcp("", write_scratch_file), 0, "test_comp_duplicate_line_for_gcp");
	assert_equal_int(c.duplicate_line_for_gcp("define x", write_scratch_file), INVALID_LINE, "test_comp_duplicate_line_for_gcp");
	assert_equal_int(c.duplicate_line_for_gcp("declaration output o", write_scratch_file), INVALID_LINE, "test_comp_duplicate_line_for_gcp");
	assert_equal_int(c.duplicate_line_for_gcp("declare input x 3", write_scratch_file), INVALID_LINE, "test_comp_duplicate_line_for_gcp");
	assert_equal_int(c.duplicate_line_for_gcp("declare in x", write_scratch_file), BAD_VAR_TYPE, "test_comp_duplicate_line_for_gcp");


	// successful declarations
	assert_equal_int(c.duplicate_line_for_gcp("declare input x", write_scratch_file), 0, "test_comp_duplicate_line_for_gcp");
	assert_equal_int(c.duplicate_line_for_gcp("declare weight w", write_scratch_file), 0, "test_comp_duplicate_line_for_gcp");
	assert_equal_int(c.duplicate_line_for_gcp("declare exp_output y", write_scratch_file), 0, "test_comp_duplicate_line_for_gcp");
	assert_equal_int(c.duplicate_line_for_gcp("declare intvar z", write_scratch_file), 0, "test_comp_duplicate_line_for_gcp");
	assert_equal_int(c.duplicate_line_for_gcp("declare output o", write_scratch_file), 0, "test_comp_duplicate_line_for_gcp");
	assert_equal_int(c.duplicate_line_for_gcp("declare loss l", write_scratch_file), 0, "test_comp_duplicate_line_for_gcp");

	// make sure variable types were changed appropriately
	string declarations[6] = {"declare input x", "declare input w", "declare input y",
		"declare intvar z", "declare intvar o", "declare intvar l"
	};
	assert_equal_file_lines("scratch.tf", declarations, 0, 6, "test_comp_duplicate_line_for_gcp");


	// succcessful definitions
	assert_equal_int(c.duplicate_line_for_gcp("define z = 3", write_scratch_file), 0, "test_comp_duplicate_line_for_gcp");
	assert_equal_int(c.duplicate_line_for_gcp("define o = z", write_scratch_file), 0, "test_comp_duplicate_line_for_gcp");
	assert_equal_int(c.duplicate_line_for_gcp("define l = pow 8 o", write_scratch_file), 0, "test_comp_duplicate_line_for_gcp");

	// make sure definition lines were directly copied
	string definitions[3] = {"define z = 3", "define o = z", "define l = pow 8 o"};
	assert_equal_file_lines("scratch.tf", definitions, 6, 3, "test_comp_duplicate_line_for_gcp");

	write_scratch_file.close();
	pass("test_comp_duplicate_line_for_gcp"); 

}

void test_comp_generate_partial_var_name() {

	assert_equal_string(generate_partial_var_name("y", ""), "", "test_comp_generate_partial_var_name");
	assert_equal_string(generate_partial_var_name("", "x"), "", "test_comp_generate_partial_var_name");
	assert_equal_string(generate_partial_var_name("y", "x"), "d/y/d/x", "test_comp_generate_partial_var_name");
	assert_equal_string(generate_partial_var_name("loss_var", "other_variable"), "d/loss_var/d/other_variable", "test_comp_generate_partial_var_name");

	pass("test_comp_generate_partial_var_name");

}

void test_comp_generate_intvar_name() {

	assert_equal_string(generate_intvar_name("", 17), "", "test_comp_generate_intvar_name");
	assert_equal_string(generate_intvar_name("foo", 17), "foo:17", "test_comp_generate_intvar_name");
	assert_equal_string(generate_intvar_name("d/lambda/d/x", 0), "d/lambda/d/x:0", "test_comp_generate_intvar_name");

	pass("test_comp_generate_intvar_name");

}

void test_comp_declare_partial_lambda() {

	Compiler c;

	ofstream write_scratch_file;


	Node *loss = new Node("lambda", false);
	loss->set_type(VariableType::LOSS);
	loss->add_parent(loss);

	Node *w = new Node("w", false);	// will later be set as weight
	Node *m = new Node("m", false);
	m->set_type(VariableType::INPUT);
	// manually "cheat" by giving W and M parents
	Node *parent = new Node("parent", false);
	m->add_parent(parent);
	w->add_parent(parent);



	// trivial errors
	assert_equal_string(c.declare_partial_lambda(w, loss, write_scratch_file), "", "test_comp_declare_partial_lambda");
	write_scratch_file.open("scratch.tf");
	assert_equal_string(c.declare_partial_lambda(NULL, loss, write_scratch_file), "", "test_comp_declare_partial_lambda");
	assert_equal_string(c.declare_partial_lambda(w, loss, write_scratch_file), "", "test_comp_declare_partial_lambda");
	w->set_type(VariableType::WEIGHT);
	assert_equal_string(c.declare_partial_lambda(w, NULL, write_scratch_file), "", "test_comp_declare_partial_lambda");

	// make sure weight partials become outputs, and others become intvars
	assert_equal_string(c.declare_partial_lambda(w, loss, write_scratch_file), "d/lambda/d/w", "test_comp_declare_partial_lambda");
	assert_equal_string(c.declare_partial_lambda(loss, loss, write_scratch_file), "d/lambda/d/lambda", "test_comp_declare_partial_lambda");
	assert_equal_string(c.declare_partial_lambda(m, loss, write_scratch_file), "d/lambda/d/m", "test_comp_declare_partial_lambda");

	string partial_declarations[3] = {"declare output d/lambda/d/w", "declare intvar d/lambda/d/lambda", "declare intvar d/lambda/d/m"};
	assert_equal_file_lines("scratch.tf", partial_declarations, 0, 3, "test_comp_declare_partial_lambda");


	write_scratch_file.close();
	delete w; delete loss; delete m; delete parent;
	pass("test_comp_declare_partial_lambda");

}

void test_comp_define_partial_lambda() {

	Compiler c;

	ofstream write_scratch_file;
	Node *node = new Node("node", false), *parent = new Node("parent", false);
	node->set_type(VariableType::INPUT); parent->set_type(VariableType::INTVAR);
	
	node->add_parent(parent);
	parent->set_child(node);
	parent->set_operation(OperationType::EXP);

	

	Node *lambda = new Node("lambda", false);
	lambda->set_type(VariableType::LOSS);
	lambda->set_child(parent);
	parent->add_parent(lambda);
	lambda->set_operation(OperationType::EXP);

	// trivial errors
	c.define_partial_lambda(NULL, "lambda", write_scratch_file, "d/lambda/d/w");
	c.define_partial_lambda(node, "", write_scratch_file, "d/lambda/d/w");
	c.define_partial_lambda(node, "lambda", write_scratch_file, "d/lambda/d/w");
	write_scratch_file.open("scratch.tf");
	c.define_partial_lambda(node, "lambda", write_scratch_file, "");

	// make sure partial(loss, loss) = 1 and partial(loss, parent) = exp parent
	c.define_partial_lambda(lambda, "lambda", write_scratch_file, "d/lambda/d/lambda");
	c.define_child_one_partial(lambda, write_scratch_file, "d/lambda/d/parent");
	c.get_visited_nodes()->insert(lambda);
	c.get_visited_node_names()->insert("lambda");

	// make sure partial(parent, node) = exp node
	c.define_child_one_partial(parent, write_scratch_file, "d/parent/d/node");
	c.get_visited_nodes()->insert(parent);
	c.get_visited_node_names()->insert("parent");
	// make sure partial(loss, node) = partial(loss, parent) * partial(parent, node)
	c.define_partial_lambda(node, "lambda", write_scratch_file, "d/lambda/d/node");

	string partial_definitions[4] = {"define d/lambda/d/lambda = 1",
		"define d/lambda/d/parent = exp parent",
		"define d/parent/d/node = exp node",
		"define d/lambda/d/node = mul d/lambda/d/parent d/parent/d/node"};
	assert_equal_file_lines("scratch.tf", partial_definitions, 0, 4, "test_comp_define_partial_lambda");


	write_scratch_file.close();
	delete node; delete parent; delete lambda;
	pass("test_comp_define_partial_lambda");

}

void test_comp_declare_child_partials() {

	Compiler comp;
	ofstream write_scratch_file;

	// trivial errors
	assert_equal_string(comp.declare_child_one_partial(NULL, write_scratch_file), "", "test_comp_declare_child_partials");
	assert_equal_string(comp.declare_child_two_partial(new Node("x", false), write_scratch_file), "", "test_comp_declare_child_partials");
	write_scratch_file.open("scratch.tf");

	// We build a node "tree" that contains all combinations of 1 or 2 constant or variable children:

	/* 

		1 	2		3
		 \ /       / 
		  A 	  B
			\   /
		  	  C 	 4
			    \   /
		  	  5	  D
			   \ /
				E
				 \
				  F

	*/

	
	Node *a = new Node("a", false);
	a->set_child(new Node("1", true)); a->set_child(new Node("2", true));

	Node *b = new Node("b", false);
	b->set_child(new Node("3", true));

	Node *c = new Node("c", false);
	c->set_child(a); c->set_child(b);

	Node *d = new Node("d", false);
	d->set_child(c); d->set_child(new Node("4", true));

	Node *e = new Node("e", false);
	e->set_child(new Node("5", true)); e->set_child(d);

	Node *f = new Node("f", false);
	f->set_child(e);

	Node *nodes[6] = {f, e, d, c, b, a};
	string child_one_partials[6] = {"d/f/d/e", "", "d/d/d/c", "d/c/d/a", "", ""};
	string child_two_partials[6] = {"", "d/e/d/d", "", "d/c/d/b", "", ""};

	for (int p = 0; p < 6; p++) {
		assert_equal_string(comp.declare_child_one_partial(nodes[p], write_scratch_file), child_one_partials[p], "test_comp_declare_child_partials");
	}

	for (int q = 0; q < 6; q++) {
		assert_equal_string(comp.declare_child_two_partial(nodes[q], write_scratch_file), child_two_partials[q], "test_comp_declare_child_partials");
	}

	string child_one_declarations[3] = {"declare intvar d/f/d/e", "declare intvar d/d/d/c", "declare intvar d/c/d/a"};
	string child_two_declarations[2] = {"declare intvar d/e/d/d", "declare intvar d/c/d/b"};
	
	assert_equal_file_lines("scratch.tf", child_one_declarations, 0, 3, "test_comp_declare_child_partials");
	assert_equal_file_lines("scratch.tf", child_two_declarations, 3, 2, "test_comp_declare_child_partials");

	write_scratch_file.close();
	delete a; delete b; delete c; delete d; delete e; delete f;
	pass("test_comp_declare_child_partials");

}

void test_comp_define_child_partials() {

	Compiler comp;
	ofstream write_scratch_file;

	// trivial errors
	comp.define_child_one_partial(NULL, write_scratch_file, "d/x/d/y");
	comp.define_child_two_partial(new Node("x", false), write_scratch_file, "d/x/d/y");
	write_scratch_file.open("scratch.tf");
	comp.define_child_one_partial(new Node("x", false), write_scratch_file, "");


	// We build a node "tree" that contains all combinations of 1 or 2 constant or variable children:

	/* 

		A 	  B	     C
		 \add/   exp/ 
		   D 	   E
			\ mul /
		  	   F 	 G
			    \pow/
		  	      H
			       \ln
			   		I
	  				 \logistic
	   	     		  J
	   	     		/   \
	   	     	    \mul/
	   	     		  K

	*/


	Node *a = new Node("a", false), *b = new Node("b", false), *c = new Node("c", false);
	Node *d = new Node("d", false), *e = new Node("e", false);
	Node *f = new Node("f", false), *g = new Node("g", false);
	Node *h = new Node("h", false);
	Node *i = new Node("i", false);
	Node *j = new Node("j", false);
	Node *k = new Node("k", false);


	d->set_child(a); d->set_child(b);
	e->set_child(c);
	f->set_child(d); f->set_child(e);
	h->set_child(f); h->set_child(g);
	i->set_child(h);
	j->set_child(i);
	k->set_child(j); k->set_child(j);

	d->set_operation(OperationType::ADD);
	e->set_operation(OperationType::EXP);
	f->set_operation(OperationType::MUL);
	h->set_operation(OperationType::POW);
	i->set_operation(OperationType::LN);
	j->set_operation(OperationType::LOGISTIC);
	k->set_operation(OperationType::MUL);

	Node *nodes[7] = {d, e, f, h, i, j, k};
	string child_one_partials[7] = {"d/d/d/a", "d/e/d/c", "d/f/d/d", "d/h/d/f", "d/i/d/h", "d/j/d/i", "d/k/d/j"};
	string child_two_partials[7] = {"d/d/d/b", "", "d/f/d/e", "d/h/d/g", "", "", "d/k/d/j"};

	for (int p = 0; p < 7; p++) {
		comp.define_child_one_partial(nodes[p], write_scratch_file, child_one_partials[p]);
	}

	for (int q = 0; q < 7; q++) {
		comp.define_child_two_partial(nodes[q], write_scratch_file, child_two_partials[q]);
	}

	string child_one_definitions[19] = {
		"define d/d/d/a = 1",

		"define d/e/d/c = exp c",

		"define d/f/d/d = e",

		"declare intvar d/h/d/f:0", "declare intvar d/h/d/f:1",
			"define d/h/d/f:0 = add -1 g", "define d/h/d/f:1 = pow f d/h/d/f:0",
			"define d/h/d/f = mul g d/h/d/f:1",

		"define d/i/d/h = pow h -1",

		"declare intvar d/j/d/i:0", "declare intvar d/j/d/i:1", "declare intvar d/j/d/i:2", "declare intvar d/j/d/i:3",
			"define d/j/d/i:0 = exp i", "define d/j/d/i:1 = add 1 d/j/d/i:0",
			"define d/j/d/i:2 = pow d/j/d/i:1 2", "define d/j/d/i:3 = pow d/j/d/i:2 -1",
			"define d/j/d/i = mul d/j/d/i:0 d/j/d/i:3",
		
		"define d/k/d/j = mul 2 j"
	};

	string child_two_definitions[8] = {
		"define d/d/d/b = 1",

		"define d/f/d/e = d",

		"declare intvar d/h/d/g:0", "declare intvar d/h/d/g:1",
		"define d/h/d/g:0 = pow f g", "define d/h/d/g:1 = ln f",
		"define d/h/d/g = mul d/h/d/g:0 d/h/d/g:1",

		"define d/k/d/j = mul 2 j"
	};
	
	assert_equal_file_lines("scratch.tf", child_one_definitions, 0, 19, "test_comp_define_child_partials");
	assert_equal_file_lines("scratch.tf", child_two_definitions, 19, 8, "test_comp_define_child_partials");

	write_scratch_file.close();
	delete a; delete b; delete c; delete d; delete e; delete f; delete g; delete h; delete i; delete j; delete k;
	pass("test_comp_define_child_partials");

}
void test_comp_define_child_two_partial() {

}


void run_comp_tests() {

	cout << "\nTesting Compiler Class... " << endl << endl;

	test_comp_constructor_compile_destructor();
	test_comp_parse_line();
	test_comp_duplicate_line_for_gcp();
	test_comp_generate_partial_var_name();
	test_comp_generate_intvar_name();
	test_comp_declare_partial_lambda();
	test_comp_define_partial_lambda();
	test_comp_declare_child_partials();
	test_comp_define_child_partials();

	cout << "\nAll Compiler Tests Passed." << endl << endl;
}
