#include <iostream>
#include <fstream>

#include "TestPreprocessor.h"
#include "../src/Preprocessor.h"
#include "TestUtilities.h"

using namespace std;


void test_pp_constructor_expand_destructor() {

	// test constructor initializes maps, set, and bool correctly
	Preprocessor *p = new Preprocessor();
	assert_equal_int(p->variables->size(), 0, "test_pp_constructor_expand_destructor");
	assert_equal_int(p->vectors->size(), 0, "test_pp_constructor_expand_destructor");
	assert_equal_int(p->vector_dimensions->size(), 0, "test_pp_constructor_expand_destructor");
	assert_equal_int(p->defined_variables->size(), 0, "test_pp_constructor_expand_destructor");
	assert_equal_int(p->macros->size(), 0, "test_pp_constructor_expand_destructor");
	assert_false(p->macros_done, "Macros_done must initially be false", "test_pp_constructor_expand_destructor");

	// preprocess the test Shape Program
	// involves all the primitive and vector operations, and macros
	// make sure the output is identical to the expected output
	assert_equal_int(p->expand_program("tests/test_files/inputs/shape_simple.tf", "tests/test_files/outputs/expanded_shape_simple.tf"), 0, "test_pp_constructor_expand_destructor");
	assert_identical_files("tests/test_files/outputs/expanded_shape_simple.tf", "tests/test_files/exp_outputs/expanded_shape_simple.tf", "test_pp_constructor_expand_destructor");

	// test destructor deletes member variables without problems
	delete p;
	pass("test_pp_constructor_expand_destructor");

}


void test_pp_expand_line() {

	Preprocessor p;
	ofstream write_scratch_file;

	// passing ofstream that isn't open
	assert_equal_int(p.expand_line("declare input x", write_scratch_file), OTHER_ERROR, "test_pp_expand_line");
	write_scratch_file.open("scratch.tf");
	// empty line
	assert_equal_int(p.expand_line("", write_scratch_file), 0, "test_pp_expand_line");

	// simple errors
	assert_equal_int(p.expand_line("declare x", write_scratch_file), INVALID_LINE, "test_pp_expand_line");
	assert_equal_int(p.expand_line("define 3", write_scratch_file), INVALID_LINE, "test_pp_expand_line");
	assert_equal_int(p.expand_line("declare_vector vec", write_scratch_file), INVALID_LINE, "test_pp_expand_line");
	assert_equal_int(p.expand_line("#macro my_macro;", write_scratch_file), INVALID_LINE, "test_pp_expand_line");
	assert_equal_int(p.expand_line("eclare intvar x", write_scratch_file), INVALID_LINE, "test_pp_expand_line");
	assert_equal_int(p.expand_line("defin x = 1", write_scratch_file), INVALID_LINE, "test_pp_expand_line");
	assert_equal_int(p.expand_line("#declare_vector input vec 3", write_scratch_file), INVALID_LINE, "test_pp_expand_line");
	assert_equal_int(p.expand_line("macro z = my_macro x; define z = x;", write_scratch_file), INVALID_LINE, "test_pp_expand_line");

	// make sure multiple macro definitions can occur sequentially with the same dummy names
	// also make sure the macros map is correctly populated
	// make sure the variables and defined variables maps are correctly cleared
	assert_equal_int(p.expand_line("#macro c = my_unary_macro a; declare intvar p; define p = ln a; define c = exp p;", write_scratch_file), 0, "test_pp_expand_line");
	assert_equal_int(p.expand_line("#macro c = my_binary_macro a b; declare intvar p; define p = mul b a; define c = logistic p;", write_scratch_file), 0, "test_pp_expand_line");
	assert_equal_int(p.macros->count("my_unary_macro"), 1, "test_pp_expand_line");
	assert_equal_int(p.macros->count("my_binary_macro"), 1, "test_pp_expand_line");
	assert_equal_int(p.variables->size(), 0, "test_pp_expand_line");
	assert_equal_int(p.defined_variables->size(), 0, "test_pp_expand_line");

	// make sure the two declarations happened successfully
	// make sure both variables are in the VARIABLES map, and that X is in DEFINED_VARIABLES
	assert_equal_int(p.expand_line("declare input x", write_scratch_file), 1, "test_pp_expand_line");
	assert_equal_int(p.expand_line("declare intvar p", write_scratch_file), 1, "test_pp_expand_line");
	assert_true(p.variables->at("x") == VariableType::INPUT, "X is an Input variable", "test_pp_expand_line");
	assert_true(p.variables->at("p") == VariableType::INTVAR, "P is an Intvar Variable", "test_pp_expand_line");
	assert_equal_int(p.defined_variables->count("x"), 1, "test_pp_expand_line");
	assert_equal_int(p.defined_variables->count("p"), 0, "test_pp_expand_line");

	// make sure the declarations got copied without expansion
	string declarations[2] = {"declare input x", "declare intvar p"};
	assert_equal_file_lines("scratch.tf", declarations, 0, 2, "test_pp_expand_line");
	
	// make sure an attempted macro definition fails
	assert_equal_int(p.expand_line("#macro c = my_macro a; define c = 3;", write_scratch_file), MACROS_NOT_AT_TOP, "test_pp_expand_line");

	// define p and make sure it gets marked as defined
	assert_equal_int(p.expand_line("define p = my_unary_macro x", write_scratch_file), 3, "test_pp_expand_line");
	assert_equal_int(p.defined_variables->count("p"), 1, "test_pp_expand_line");

	// make sure the two vector declarations happened successfully
	// make sure both vectors are in the VECTORS map, and that INPUT_VEC is in DEFINED_VARIABLES
	assert_equal_int(p.expand_line("declare_vector input input_vec 3", write_scratch_file), 3, "test_pp_expand_line");
	assert_equal_int(p.expand_line("declare_vector output output_vec 3", write_scratch_file), 3, "test_pp_expand_line");
	assert_true(p.vectors->at("input_vec") == VariableType::INPUT, "INPUT_VEC is an Input vector", "test_pp_expand_line");
	assert_true(p.vectors->at("output_vec") == VariableType::OUTPUT, "OUTPUT_VEC is an Output Vector", "test_pp_expand_line");
	assert_equal_int(p.defined_variables->count("input_vec"), 1, "test_pp_expand_line");
	assert_equal_int(p.defined_variables->count("output_vec"), 0, "test_pp_expand_line");

	// define the vector OUTPUT_VEC. Make sure it gets marked as defined
	assert_equal_int(p.expand_line("define_vector output_vec = mul input_vec p", write_scratch_file), 3, "test_pp_expand_line");
	assert_equal_int(p.defined_variables->count("output_vec"), 1, "test_pp_expand_line");

	write_scratch_file.close();
	pass("test_pp_expand_line");


}

void test_pp_expand_declare_vector() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");
	assert_equal_int(p.expand_declare_vector_instruction("declare_vector input x 3", write_scratch_file), 3,
		"test_pp_expand_declare_vector");
	assert_equal_int(p.expand_declare_vector_instruction("declare_vector output o 5", write_scratch_file), 5,
		"test_pp_expand_declare_vector");

	write_scratch_file.close();

	ifstream read_scratch_file("scratch.tf");
	string line;

	for (int i = 0; i < 3; i++) {
		getline(read_scratch_file, line);
		assert_equal_string(line, "declare input x." + to_string(i), "test_pp_expand_declare_vector");
	}

	for (int i = 0; i < 5; i++) {
		getline(read_scratch_file, line);
		assert_equal_string(line, "declare output o." + to_string(i), "test_pp_expand_declare_vector");
	}
	
	read_scratch_file.close();

	pass("test_pp_expand_declare_vector");

}

void test_pp_expand_define() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// declare variables
	assert_equal_int(p.expand_line("declare input x", write_scratch_file), 1, "test_pp_expand_define");
	assert_equal_int(p.expand_line("declare intvar y", write_scratch_file), 1, "test_pp_expand_define");
	assert_equal_int(p.expand_line("declare output z", write_scratch_file), 1, "test_pp_expand_define");
	assert_equal_int(p.expand_line("declare intvar a", write_scratch_file), 1, "test_pp_expand_define");
	assert_equal_int(p.expand_line("declare intvar b", write_scratch_file), 1, "test_pp_expand_define");

	// define as constant, binary operation, unary operation, equivalent to another variable
	string lines[4] = {"define y = 3", "define z = sub y x", "define a = logistic z", "define b = a"};
	for (int i = 0; i < 4; i++) {
		assert_equal_int(p.expand_define_instruction(lines[i], write_scratch_file), 1, "test_pp_expand_define");
	}

	write_scratch_file.close();

	// make sure the definitions are copied without expansion
	assert_equal_file_lines("scratch.tf", lines, 5, 4, "test_pp_expand_define");

	pass("test_pp_expand_define");

}

void test_pp_expand_define_vector() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// define two macros which will be used later in this test
	p.expand_line("#macro c = my_binary_macro a b; declare intvar p; define p = pow a b; define c = ln p", write_scratch_file);
	p.expand_line("#macro z = my_unary_macro x; declare intvar q; define z = logistic x", write_scratch_file);

	// declare variables
	assert_equal_int(p.expand_line("declare_vector input x 3", write_scratch_file), 3, "test_pp_expand_define_vector");
	assert_equal_int(p.expand_line("declare_vector exp_output y 3", write_scratch_file), 3, "test_pp_expand_define_vector");
	assert_equal_int(p.expand_line("declare_vector weight w 3", write_scratch_file), 3, "test_pp_expand_define_vector");
	
	assert_equal_int(p.expand_line("declare_vector intvar a 3", write_scratch_file), 3, "test_pp_expand_define_vector");
	assert_equal_int(p.expand_line("declare_vector intvar b 3", write_scratch_file), 3, "test_pp_expand_define_vector");
	assert_equal_int(p.expand_line("declare_vector intvar c 3", write_scratch_file), 3, "test_pp_expand_define_vector");
	assert_equal_int(p.expand_line("declare_vector intvar d 3", write_scratch_file), 3, "test_pp_expand_define_vector");
	assert_equal_int(p.expand_line("declare_vector intvar e 3", write_scratch_file), 3, "test_pp_expand_define_vector");
	assert_equal_int(p.expand_line("declare_vector intvar f 3", write_scratch_file), 3, "test_pp_expand_define_vector");
	assert_equal_int(p.expand_line("declare_vector intvar g 3", write_scratch_file), 3, "test_pp_expand_define_vector");
	assert_equal_int(p.expand_line("declare_vector intvar h 3", write_scratch_file), 3, "test_pp_expand_define_vector");

	assert_equal_int(p.expand_line("declare input p", write_scratch_file), 1, "test_pp_expand_define_vector");
	assert_equal_int(p.expand_line("declare input q", write_scratch_file), 1, "test_pp_expand_define_vector");



	// binary primitive on two vectors
	assert_equal_int(p.expand_define_vector_instruction("define_vector a = add x w", write_scratch_file), 3, "test_pp_expand_define_vector");
	string addxw[3] = {"define a.0 = add x.0 w.0", "define a.1 = add x.1 w.1", "define a.2 = add x.2 w.2"};
	assert_equal_file_lines("scratch.tf", addxw, 35, 3, "test_pp_expand_define_vector");
	
	// binary primitive on a vector and a variable
	assert_equal_int(p.expand_define_vector_instruction("define_vector b = mul a p", write_scratch_file), 3, "test_pp_expand_define_vector");
	string mulap[3] = {"define b.0 = mul a.0 p", "define b.1 = mul a.1 p", "define b.2 = mul a.2 p"};
	assert_equal_file_lines("scratch.tf", mulap, 38, 3, "test_pp_expand_define_vector");
	
	// binary primitive on a vector and a constant
	assert_equal_int(p.expand_define_vector_instruction("define_vector c = pow b 2", write_scratch_file), 3, "test_pp_expand_define_vector");
	string powb2[3] = {"define c.0 = pow b.0 2", "define c.1 = pow b.1 2", "define c.2 = pow b.2 2"};
	assert_equal_file_lines("scratch.tf", powb2, 41, 3, "test_pp_expand_define_vector");
	


	// binary macro on two vectors
	assert_equal_int(p.expand_define_vector_instruction("define_vector d = my_binary_macro x w", write_scratch_file), 3, "test_pp_expand_define_vector");
	string macroxw[9] = {
		"declare intvar d.0_p0", "define d.0_p0 = pow x.0 w.0", "define d.0 = ln d.0_p0",
		"declare intvar d.1_p1", "define d.1_p1 = pow x.1 w.1", "define d.1 = ln d.1_p1",
		"declare intvar d.2_p2", "define d.2_p2 = pow x.2 w.2", "define d.2 = ln d.2_p2"
	};
	assert_equal_file_lines("scratch.tf", macroxw, 44, 9, "test_pp_expand_define_vector");
	
	// binary macro on a vector and a variable
	assert_equal_int(p.expand_define_vector_instruction("define_vector e = my_binary_macro d q", write_scratch_file), 3, "test_pp_expand_define_vector");
	string macrodq[9] = {
		"declare intvar e.0_p3", "define e.0_p3 = pow d.0 q", "define e.0 = ln e.0_p3",
		"declare intvar e.1_p4", "define e.1_p4 = pow d.1 q", "define e.1 = ln e.1_p4",
		"declare intvar e.2_p5", "define e.2_p5 = pow d.2 q", "define e.2 = ln e.2_p5"
	};
	assert_equal_file_lines("scratch.tf", macrodq, 53, 9, "test_pp_expand_define_vector");
	
	// binary macro on a vector and a constant
	assert_equal_int(p.expand_define_vector_instruction("define_vector f = my_binary_macro e -3", write_scratch_file), 3, "test_pp_expand_define_vector");
	string macroe3[9] = {
		"declare intvar f.0_p6", "define f.0_p6 = pow e.0 -3", "define f.0 = ln f.0_p6",
		"declare intvar f.1_p7", "define f.1_p7 = pow e.1 -3", "define f.1 = ln f.1_p7",
		"declare intvar f.2_p8", "define f.2_p8 = pow e.2 -3", "define f.2 = ln f.2_p8"
	};
	assert_equal_file_lines("scratch.tf", macroe3, 62, 9, "test_pp_expand_define_vector");



	// unary primitive on a vector
	assert_equal_int(p.expand_define_vector_instruction("define_vector g = logistic x", write_scratch_file), 3, "test_pp_expand_define_vector");
	string logx[3] = {"define g.0 = logistic x.0", "define g.1 = logistic x.1", "define g.2 = logistic x.2"};
	assert_equal_file_lines("scratch.tf", logx, 71, 3, "test_pp_expand_define_vector");


	// unary macro on a vector
	assert_equal_int(p.expand_define_vector_instruction("define_vector h = my_unary_macro g", write_scratch_file), 3, "test_pp_expand_define_vector");
	string macrog[9] = {
		"declare intvar h.0_q0", "define h.0 = logistic g.0", 
		"declare intvar h.1_q1", "define h.1 = logistic g.1",
		"declare intvar h.2_q2", "define h.2 = logistic g.2",
	};
	assert_equal_file_lines("scratch.tf", macrog, 74, 6, "test_pp_expand_define_vector");

	write_scratch_file.close();
	pass("test_pp_expand_define_vector");

}

void test_pp_expand_vector_operation() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// declare vectors
	assert_equal_int(p.expand_line("declare_vector input x 3", write_scratch_file), 3, "test_pp_expand_vector_operation");
	assert_equal_int(p.expand_line("declare_vector weight w 3", write_scratch_file), 3, "test_pp_expand_vector_operation");
	assert_equal_int(p.expand_line("declare_vector intvar u 3", write_scratch_file), 3, "test_pp_expand_vector_operation");
	assert_equal_int(p.expand_line("declare_vector intvar v 3", write_scratch_file), 3, "test_pp_expand_vector_operation");
	assert_equal_int(p.expand_line("declare_vector output a 3", write_scratch_file), 3, "test_pp_expand_vector_operation");
	assert_equal_int(p.expand_line("declare_vector output b 3", write_scratch_file), 3, "test_pp_expand_vector_operation");

	// declare z, a scalar which will be defined as a dot product
	// declare a scalar i, used as an incrementing factor in the definition of v
	assert_equal_int(p.expand_line("declare intvar z", write_scratch_file), 1, "test_pp_expand_vector_operation");
	assert_equal_int(p.expand_line("declare input i", write_scratch_file), 1, "test_pp_expand_vector_operation");
	
	// define z as the dot product of x and w
	assert_equal_int(p.expand_vector_operation(OperationType::DOT, "z", "x", "w", write_scratch_file), 11,
		"test_pp_expand_vector_operation");

	// define u as a scaled vector
	assert_equal_int(p.expand_vector_operation(OperationType::SCALE_VECTOR, "u", "x", "-2", write_scratch_file), 3,
		"test_pp_expand_vector_operation");

	// define v as an incremented vector
	assert_equal_int(p.expand_vector_operation(OperationType::INCREMENT_VECTOR, "v", "w", "i", write_scratch_file), 3,
		"test_pp_expand_vector_operation");

	// define a as a component wise sum vector
	assert_equal_int(p.expand_vector_operation(OperationType::COMPONENT_WISE_ADD, "a", "x", "v", write_scratch_file), 3,
		"test_pp_expand_vector_operation");

	// define b as a component wise product vector
	assert_equal_int(p.expand_vector_operation(OperationType::COMPONENT_WISE_MUL, "b", "u", "a", write_scratch_file), 3,
		"test_pp_expand_vector_operation");


	write_scratch_file.close();

	// make sure the dot product expansion is correct
	string dot_expansion[11] = {"declare intvar z.0", "define z.0 = mul x.0 w.0",
		"declare intvar z.1", "define z.1 = mul x.1 w.1",
		"declare intvar z.2", "define z.2 = mul x.2 w.2",
		"declare intvar z.3", "define z.3 = add z.0 z.1",
		"declare intvar z.4", "define z.4 = add z.3 z.2",
		"define z = z.4"
	};
	assert_equal_file_lines("scratch.tf", dot_expansion, 20, 11, "test_pp_expand_vector_operation");

	// make sure the scaled vector expansion is correct
	string scaled_vec_expansion[3] = {"define u.0 = mul x.0 -2", "define u.1 = mul x.1 -2", "define u.2 = mul x.2 -2"};
	assert_equal_file_lines("scratch.tf", scaled_vec_expansion, 31, 3, "test_pp_expand_vector_operation");

	// make sure the incremented vector expansion is correct
	string incr_vec_expansion[3] = {"define v.0 = add w.0 i", "define v.1 = add w.1 i", "define v.2 = add w.2 i"};
	assert_equal_file_lines("scratch.tf", incr_vec_expansion, 34, 3, "test_pp_expand_vector_operation");
	
	// make sure the component wise add expansion is correct
	string comp_wise_add_expansion[3] = {"define a.0 = add x.0 v.0", "define a.1 = add x.1 v.1", "define a.2 = add x.2 v.2"};
	assert_equal_file_lines("scratch.tf", comp_wise_add_expansion, 37, 3, "test_pp_expand_vector_operation");
	
	// make sure the component wise mul expansion is correct
	string comp_wise_mul_expansion[3] = {"define b.0 = mul u.0 a.0", "define b.1 = mul u.1 a.1", "define b.2 = mul u.2 a.2"};
	assert_equal_file_lines("scratch.tf", comp_wise_mul_expansion, 40, 3, "test_pp_expand_vector_operation");
	
	pass("test_pp_expand_vector_operation");

}

void test_pp_expand_unary_macro() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// manually create a macro struct
	// parsing of a #macro line will be tested in another function
	struct macro *m = new struct macro();
	m->name = "my_macro"; m->num_lines = 3; m->is_binary = false;
	m->result = "c"; m->operand1 = "a"; m->operand2 = "";
	m->lines = new vector<string>(); m->lines->push_back("declare intvar b");
	m->lines->push_back("define b = add a 3"); m->lines->push_back("define c = mul b b");
	m->num_references = 0;
	p.macros->insert(make_pair("my_macro", m));

	// declare variables
	assert_equal_int(p.expand_line("declare intvar x", write_scratch_file), 1, "test_pp_expand_unary_macro");
	assert_equal_int(p.expand_line("declare intvar z", write_scratch_file), 1, "test_pp_expand_unary_macro");
	assert_equal_int(p.expand_line("declare output o", write_scratch_file), 1, "test_pp_expand_unary_macro");
	
	// define z as my_macro(x), and o as my_macro(z)
	assert_equal_int(p.expand_unary_macro("my_macro", "x", "z", write_scratch_file), 3, "test_pp_expand_unary_macro");
	assert_equal_int(p.expand_unary_macro("my_macro", "z", "o", write_scratch_file), 3, "test_pp_expand_unary_macro");
	write_scratch_file.close();

	// make sure the definition of z expands correctly
	string z_expansion[3] = {"declare intvar z_b0", "define z_b0 = add x 3", "define z = mul z_b0 z_b0"};
	assert_equal_file_lines("scratch.tf", z_expansion, 3, 3, "test_pp_expand_unary_macro");

	// make sure the definition of o expands correctly
	string o_expansion[3] = {"declare intvar o_b1", "define o_b1 = add z 3", "define o = mul o_b1 o_b1"};
	assert_equal_file_lines("scratch.tf", o_expansion, 6, 3, "test_pp_expand_unary_macro");

	pass("test_pp_expand_unary_macro");

}

void test_pp_expand_binary_macro() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// manually create a macro struct
	// parsing of a #macro line will be tested in another function
	struct macro *m = new struct macro();
	m->name = "my_macro"; m->num_lines = 3; m->is_binary = true;
	m->result = "c"; m->operand1 = "a"; m->operand2 = "b";
	m->lines = new vector<string>();
	m->lines->push_back("declare intvar q"); m->lines->push_back("define q = ln a"); m->lines->push_back("define c = pow b q");
	m->num_references = 0;
	p.macros->insert(make_pair("my_macro", m));

	// declare variables
	assert_equal_int(p.expand_line("declare intvar x", write_scratch_file), 1, "test_pp_expand_binary_macro");
	assert_equal_int(p.expand_line("declare input y", write_scratch_file), 1, "test_pp_expand_binary_macro");
	assert_equal_int(p.expand_line("declare intvar z", write_scratch_file), 1, "test_pp_expand_binary_macro");
	assert_equal_int(p.expand_line("declare output o", write_scratch_file), 1, "test_pp_expand_binary_macro");
	
	// define z as my_macro(x), and o as my_macro(z)
	assert_equal_int(p.expand_binary_macro("my_macro", "x", "y", "z", write_scratch_file), 3, "test_pp_expand_binary_macro");
	assert_equal_int(p.expand_binary_macro("my_macro", "z", "y", "o", write_scratch_file), 3, "test_pp_expand_binary_macro");
	write_scratch_file.close();

	// make sure the definition of z expands correctly
	string z_expansion[3] = {"declare intvar z_q0", "define z_q0 = ln x", "define z = pow y z_q0"};
	assert_equal_file_lines("scratch.tf", z_expansion, 4, 3, "test_pp_expand_binary_macro");

	// make sure the definition of o expands correctly
	string o_expansion[3] = {"declare intvar o_q1", "define o_q1 = ln z", "define o = pow y o_q1"};
	assert_equal_file_lines("scratch.tf", o_expansion, 7, 3, "test_pp_expand_binary_macro");

	pass("test_pp_expand_binary_macro");
}

void test_pp_substitute_dummy_names() {

	Preprocessor p;

	// test substituting of dummy names for this macro:
	// 	#macro c = my_macro a b; declare intvar q; define q = pow a b; declare intvar p; define p = exp q; define c = ln p
	// the substitution happens during the expansion of the line "define z = my_macro x y"
	assert_equal_string(p.substitute_dummy_names("declare intvar q", "c", "a", "b", "z", "x", "y", 0),
		"declare intvar z_q0", "test_pp_substitute_dummy_names");
	assert_equal_string(p.substitute_dummy_names("define q = pow a b", "c", "a", "b", "z", "x", "y", 0),
		"define z_q0 = pow x y", "test_pp_substitute_dummy_names");
	assert_equal_string(p.substitute_dummy_names("declare intvar p", "c", "a", "b", "z", "x", "y", 0),
		"declare intvar z_p0", "test_pp_substitute_dummy_names");
	assert_equal_string(p.substitute_dummy_names("define p = exp q", "c", "a", "b", "z", "x", "y", 0),
		"define z_p0 = exp z_q0", "test_pp_substitute_dummy_names");
	assert_equal_string(p.substitute_dummy_names("define c = ln p", "c", "a", "b", "z", "x", "y", 0),
		"define z = ln z_p0", "test_pp_substitute_dummy_names");

	// test substituting of dummy names for this macro:
	// 	#macro c = my_macro a b; declare intvar q; define q = logistic b; declare intvar x; define x = mul q a; define c = add x b
	// the substitution happens during the expansion of the line "define z = my_macro x a"
	assert_equal_string(p.substitute_dummy_names("declare intvar q", "c", "a", "b", "z", "x", "a", 1),
		"declare intvar z_q1", "test_pp_substitute_dummy_names");
	assert_equal_string(p.substitute_dummy_names("define q = logistic b", "c", "a", "b", "z", "x", "a", 1),
		"define z_q1 = logistic a", "test_pp_substitute_dummy_names");
	assert_equal_string(p.substitute_dummy_names("declare intvar x", "c", "a", "b", "z", "x", "a", 1),
		"declare intvar z_x1", "test_pp_substitute_dummy_names");
	assert_equal_string(p.substitute_dummy_names("define x = mul q a", "c", "a", "b", "z", "x", "a", 1),
		"define z_x1 = mul z_q1 x", "test_pp_substitute_dummy_names");
	assert_equal_string(p.substitute_dummy_names("define c = add x b", "c", "a", "b", "z", "x", "a", 1),
		"define z = add z_x1 a", "test_pp_substitute_dummy_names");


	pass("test_pp_substitute_dummy_names");

}

void test_pp_macro_validation() {

	Preprocessor p;

	// manually create a binary macro struct
	// parsing of a #macro line will be tested in another function
	struct macro *bm = new struct macro();
	bm->name = "my_binary_macro"; bm->num_lines = 3; bm->is_binary = true;
	bm->result = "c"; bm->operand1 = "a"; bm->operand2 = "b";
	bm->lines = new vector<string>();
	bm->lines->push_back("declare intvar q"); bm->lines->push_back("define q = ln a"); bm->lines->push_back("define c = pow b q");
	bm->num_references = 0;
	p.macros->insert(make_pair("my_binary_macro", bm));

	// create another macro struct, this time unary
	struct macro *um = new struct macro();
	um->name = "my_unary_macro"; um->num_lines = 3; um->is_binary = false;
	um->result = "c"; um->operand1 = "a"; um->operand2 = "";
	um->lines = new vector<string>();
	um->lines->push_back("declare intvar q"); um->lines->push_back("define q = ln a"); um->lines->push_back("define c = pow q q");
	um->num_references = 0;
	p.macros->insert(make_pair("my_unary_macro", um));

	// make sure the unary and binary macros are both valid
	// make sure a non-existent macro is invalid
	assert_true(p.is_valid_macro("my_binary_macro"), "Should be a valid macro", "test_pp_macro_validation");
	assert_true(p.is_valid_macro("my_unary_macro"), "Should be a valid macro", "test_pp_macro_validation");
	assert_false(p.is_valid_macro("non_existent_macro"), "Should not be a valid macro", "test_pp_macro_validation");

	// make sure the first macro is binary, and the second not
	assert_true(p.is_binary_macro("my_binary_macro"), "Should be a binary macro", "test_pp_macro_validation");
	assert_false(p.is_unary_macro("my_binary_macro"), "Should not be a unary macro", "test_pp_macro_validation");
	assert_true(p.is_unary_macro("my_unary_macro"), "Should be a unary macro", "test_pp_macro_validation");
	assert_false(p.is_binary_macro("my_unary_macro"), "Should not be a binary macro", "test_pp_macro_validation");

	pass("test_pp_macro_validation");

}

void test_pp_expand_dot_product() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// declare vectors
	assert_equal_int(p.expand_line("declare_vector input x 3", write_scratch_file), 3, "test_pp_expand_dot_product");
	assert_equal_int(p.expand_line("declare_vector weight w 3", write_scratch_file), 3, "test_pp_expand_dot_product");
	assert_equal_int(p.expand_line("declare_vector intvar u 1", write_scratch_file), 1, "test_pp_expand_dot_product");
	assert_equal_int(p.expand_line("declare_vector intvar v 1", write_scratch_file), 1, "test_pp_expand_dot_product");

	// declare z and a, scalars which will be defined as dot products
	assert_equal_int(p.expand_line("declare intvar z", write_scratch_file), 1, "test_pp_expand_dot_product");
	assert_equal_int(p.expand_line("declare output a", write_scratch_file), 1, "test_pp_expand_dot_product");
	
	// define z as the dot product of x and w
	assert_equal_int(p.expand_dot_product_instruction("z", "x", "w", 3, write_scratch_file), 11,
		"test_pp_expand_dot_product");

	// define a as the dot product of u and v (dimension-1 vectors)
	assert_equal_int(p.expand_dot_product_instruction("a", "u", "v", 1, write_scratch_file), 3,
		"test_pp_expand_dot_product");


	write_scratch_file.close();

	// make sure the z dot product expansion is correct
	string z_expansion[11] = {"declare intvar z.0", "define z.0 = mul x.0 w.0",
		"declare intvar z.1", "define z.1 = mul x.1 w.1",
		"declare intvar z.2", "define z.2 = mul x.2 w.2",
		"declare intvar z.3", "define z.3 = add z.0 z.1",
		"declare intvar z.4", "define z.4 = add z.3 z.2",
		"define z = z.4"
	};
	assert_equal_file_lines("scratch.tf", z_expansion, 10, 11, "test_pp_expand_vector_operation");

	// make sure the a dot product expansion is correct
	string a_expansion[3] = {"declare intvar a.0", "define a.0 = mul u.0 v.0", "define a = a.0"};
	assert_equal_file_lines("scratch.tf", a_expansion, 21, 3, "test_pp_expand_vector_operation");

	pass("test_pp_expand_dot_product");

}

void test_pp_expand_vector_add() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// declare vectors
	assert_equal_int(p.expand_line("declare_vector input x 3", write_scratch_file), 3, "test_pp_expand_vector_add");
	assert_equal_int(p.expand_line("declare_vector weight w 3", write_scratch_file), 3, "test_pp_expand_vector_add");
	assert_equal_int(p.expand_line("declare_vector intvar u 3", write_scratch_file), 3, "test_pp_expand_vector_add");

	assert_equal_int(p.expand_line("declare_vector input a 3", write_scratch_file), 3, "test_pp_expand_vector_add");
	assert_equal_int(p.expand_line("declare_vector intvar b 3", write_scratch_file), 3, "test_pp_expand_vector_add");
	assert_equal_int(p.expand_line("declare_vector output v 3", write_scratch_file), 3, "test_pp_expand_vector_add");

	// define u and v as component wise sum vectors
	assert_equal_int(p.expand_component_wise_add_instruction("u", "x", "w", 3, write_scratch_file), 3, "test_pp_expand_vector_add");
	assert_equal_int(p.expand_component_wise_add_instruction("v", "a", "b", 1, write_scratch_file), 1, "test_pp_expand_vector_add");

	write_scratch_file.close();

	// make sure the U component wise add expansion is correct
	string u_expansion[3] = {"define u.0 = add x.0 w.0", "define u.1 = add x.1 w.1", "define u.2 = add x.2 w.2"};
	assert_equal_file_lines("scratch.tf", u_expansion, 18, 3, "test_pp_expand_vector_operation");

	// make sure the V component wise add expansion is correct
	string v_expansion[1] = {"define v.0 = add a.0 b.0"};
	assert_equal_file_lines("scratch.tf", v_expansion, 21, 1, "test_pp_expand_vector_operation");
	
	pass("test_pp_expand_vector_add");

}

void test_pp_expand_vector_mul() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// declare vectors
	assert_equal_int(p.expand_line("declare_vector input x 3", write_scratch_file), 3, "test_pp_expand_vector_mul");
	assert_equal_int(p.expand_line("declare_vector weight w 3", write_scratch_file), 3, "test_pp_expand_vector_mul");
	assert_equal_int(p.expand_line("declare_vector intvar u 3", write_scratch_file), 3, "test_pp_expand_vector_mul");

	assert_equal_int(p.expand_line("declare_vector input a 3", write_scratch_file), 3, "test_pp_expand_vector_mul");
	assert_equal_int(p.expand_line("declare_vector intvar b 3", write_scratch_file), 3, "test_pp_expand_vector_mul");
	assert_equal_int(p.expand_line("declare_vector output v 3", write_scratch_file), 3, "test_pp_expand_vector_mul");

	// define u and v as component wise product vectors
	assert_equal_int(p.expand_component_wise_mul_instruction("u", "x", "w", 3, write_scratch_file), 3, "test_pp_expand_vector_mul");
	assert_equal_int(p.expand_component_wise_mul_instruction("v", "a", "b", 1, write_scratch_file), 1, "test_pp_expand_vector_mul");

	write_scratch_file.close();

	// make sure the U component wise add expansion is correct
	string u_expansion[3] = {"define u.0 = mul x.0 w.0", "define u.1 = mul x.1 w.1", "define u.2 = mul x.2 w.2"};
	assert_equal_file_lines("scratch.tf", u_expansion, 18, 3, "test_pp_expand_vector_mul");

	// make sure the V component wise add expansion is correct
	string v_expansion[1] = {"define v.0 = mul a.0 b.0"};
	assert_equal_file_lines("scratch.tf", v_expansion, 21, 1, "test_pp_expand_vector_mul");
	
	pass("test_pp_expand_vector_mul");

}

void test_pp_expand_vector_scale() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// declare vectors
	// declare vectors
	assert_equal_int(p.expand_line("declare_vector input x 3", write_scratch_file), 3, "test_pp_expand_vector_scale");
	assert_equal_int(p.expand_line("declare_vector output p 3", write_scratch_file), 3, "test_pp_expand_vector_scale");
	assert_equal_int(p.expand_line("declare_vector weight y 1", write_scratch_file), 1, "test_pp_expand_vector_scale");
	assert_equal_int(p.expand_line("declare_vector intvar q 1", write_scratch_file), 1, "test_pp_expand_vector_scale");

	// define s as a scaling factor variable
	assert_equal_int(p.expand_line("declare intvar s", write_scratch_file), 1, "test_pp_expand_vector_scale");
	assert_equal_int(p.expand_line("define s = 0", write_scratch_file), 1, "test_pp_expand_vector_scale");

	// define p and q as scaled vectors
	assert_equal_int(p.expand_scale_vector_instruction("p", "x", "-2", 3, write_scratch_file), 3,
		"test_pp_expand_vector_scale");
	assert_equal_int(p.expand_scale_vector_instruction("q", "y", "s", 1, write_scratch_file), 1,
		"test_pp_expand_vector_scale");


	write_scratch_file.close();

	// make sure the p scaled vector expansion is correct
	string p_scaled_vec_expansion[3] = {"define p.0 = mul x.0 -2", "define p.1 = mul x.1 -2", "define p.2 = mul x.2 -2"};
	assert_equal_file_lines("scratch.tf", p_scaled_vec_expansion, 10, 3, "test_pp_expand_vector_scale");

	// make sure the p scaled vector expansion is correct
	string q_scaled_vec_expansion[1] = {"define q.0 = mul y.0 s"};
	assert_equal_file_lines("scratch.tf", q_scaled_vec_expansion, 13, 1, "test_pp_expand_vector_scale");

	pass("test_pp_expand_vector_scale");

}

void test_pp_expand_vector_increment() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// declare vectors
	assert_equal_int(p.expand_line("declare_vector input x 3", write_scratch_file), 3, "test_pp_expand_vector_increment");
	assert_equal_int(p.expand_line("declare_vector output p 3", write_scratch_file), 3, "test_pp_expand_vector_increment");
	assert_equal_int(p.expand_line("declare_vector weight y 1", write_scratch_file), 1, "test_pp_expand_vector_increment");
	assert_equal_int(p.expand_line("declare_vector intvar q 1", write_scratch_file), 1, "test_pp_expand_vector_increment");

	// define i as an incrementing factor variable
	assert_equal_int(p.expand_line("declare intvar i", write_scratch_file), 1, "test_pp_expand_vector_increment");
	assert_equal_int(p.expand_line("define i = 0", write_scratch_file), 1, "test_pp_expand_vector_increment");

	// define p and q as scaled vectors
	assert_equal_int(p.expand_increment_vector_instruction("p", "x", "-2", 3, write_scratch_file), 3,
		"test_pp_expand_vector_increment");
	assert_equal_int(p.expand_increment_vector_instruction("q", "y", "i", 1, write_scratch_file), 1,
		"test_pp_expand_vector_increment");

	write_scratch_file.close();

	// make sure the p incremented vector expansion is correct
	string p_incr_vec_expansion[3] = {"define p.0 = add x.0 -2", "define p.1 = add x.1 -2", "define p.2 = add x.2 -2"};
	assert_equal_file_lines("scratch.tf", p_incr_vec_expansion, 10, 3, "test_pp_expand_vector_increment");

	// make sure the p incremented vector expansion is correct
	string q_incr_vec_expansion[1] = {"define q.0 = add y.0 i"};
	assert_equal_file_lines("scratch.tf", q_incr_vec_expansion, 13, 1, "test_pp_expand_vector_increment");

	pass("test_pp_expand_vector_increment");
}

void test_pp_parse_macro_line() {

	Preprocessor p;
	struct macro *macro = new struct macro();

	// simple errors
	assert_equal_int(p.parse_macro_line("foo", NULL), OTHER_ERROR, "test_pp_parse_macro_line");
	assert_equal_int(p.parse_macro_line("", macro), 0, "test_pp_parse_macro_line");
	assert_equal_int(p.parse_macro_line("#macro c = my_macro a;", macro), INVALID_LINE, "test_pp_parse_macro_line");

	// make sure parse_macro_line correctly populates the fields of the macro struct
	assert_equal_int(p.parse_macro_line("#macro c = my_macro a b; declare intvar p; define p = ln a; define c = add p b;", macro),
		0, "test_pp_parse_macro_line");
	assert_equal_int(macro->num_lines, 3, "test_pp_parse_macro_line");
	assert_equal_int(macro->num_references, 0, "test_pp_parse_macro_line");
	assert_true(macro->is_binary, "My_macro is binary", "test_pp_parse_macro_line");
	assert_equal_string(macro->name, "my_macro", "test_pp_parse_macro_line");
	assert_equal_string(macro->result, "c", "test_pp_parse_macro_line");
	assert_equal_string(macro->operand1, "a", "test_pp_parse_macro_line");
	assert_equal_string(macro->operand2, "b", "test_pp_parse_macro_line");
	assert_equal_string(macro->lines->at(0), " declare intvar p", "test_pp_parse_macro_line");
	assert_equal_string(macro->lines->at(1), " define p = ln a", "test_pp_parse_macro_line");
	assert_equal_string(macro->lines->at(2), " define c = add p b", "test_pp_parse_macro_line");

	pass("test_pp_parse_macro_line");

}


void test_pp_parse_macro_first_line() {

	Preprocessor p;
	struct macro *macro = new struct macro();

	// trivial errors
	assert_equal_int(p.parse_macro_first_line("#macro c = my_macro a b", NULL), OTHER_ERROR, "test_pp_parse_macro_first_line");
	assert_equal_int(p.parse_macro_first_line("", macro), 0, "test_pp_parse_macro_first_line");

	// simple errors
	assert_equal_int(p.parse_macro_first_line("#macro c = my_macro", macro), INVALID_LINE, "test_pp_parse_macro_first_line");
	assert_equal_int(p.parse_macro_first_line("#macro c my_macro a b", macro), INVALID_LINE, "test_pp_parse_macro_first_line");
	assert_equal_int(p.parse_macro_first_line("#macro c = my_macro a b c", macro), INVALID_LINE, "test_pp_parse_macro_first_line");
	assert_equal_int(p.parse_macro_first_line("macro c = my_macro a b", macro), INVALID_LINE, "test_pp_parse_macro_first_line");
	assert_equal_int(p.parse_macro_first_line("#macro add = my_macro a b", macro), INVALID_VAR_NAME, "test_pp_parse_macro_first_line");
	assert_equal_int(p.parse_macro_first_line("#macro c = logistic a b", macro), INVALID_MACRO_NAME, "test_pp_parse_macro_first_line");
	assert_equal_int(p.parse_macro_first_line("#macro c = my_macro c", macro), INVALID_LINE, "test_pp_parse_macro_first_line");
	assert_equal_int(p.parse_macro_first_line("#macro c = my_macro a c", macro), INVALID_LINE, "test_pp_parse_macro_first_line");
	assert_equal_int(p.parse_macro_first_line("#macro c = my_macro ln c", macro), INVALID_LINE, "test_pp_parse_macro_first_line");
	assert_equal_int(p.parse_macro_first_line("#macro c = my_macro a exp", macro), INVALID_LINE, "test_pp_parse_macro_first_line");

	// correct unary macro
	assert_equal_int(p.parse_macro_first_line("#macro c = my_macro a", macro), 0, "test_pp_parse_macro_first_line");
	assert_equal_string(macro->name, "my_macro", "test_pp_parse_macro_first_line");
	assert_equal_string(macro->result, "c", "test_pp_parse_macro_first_line");
	assert_equal_string(macro->operand1, "a", "test_pp_parse_macro_first_line");
	assert_equal_string(macro->operand2, "", "test_pp_parse_macro_first_line");
	assert_false(macro->is_binary, "Macro should not be binary", "test_pp_parse_macro_first_line");
	
	// C and A should both be in the temporary variables set
	assert_equal_int(p.variables->size(), 2, "test_pp_parse_macro_first_line");
	assert_equal_int(p.variables->count("c"), 1, "test_pp_parse_macro_first_line");
	assert_equal_int(p.variables->count("a"), 1, "test_pp_parse_macro_first_line");

	// only the dummy operand A should be in the temporary defined variables set
	assert_equal_int(p.defined_variables->size(), 1, "test_pp_parse_macro_first_line");
	assert_equal_int(p.defined_variables->count("a"), 1, "test_pp_parse_macro_subsequent_line");
	

	// correct binary macro
	Preprocessor p2;
	struct macro *macro2 = new struct macro();
	assert_equal_int(p2.parse_macro_first_line("#macro c = my_macro a b", macro), 0, "test_pp_parse_macro_first_line");
	assert_equal_string(macro->name, "my_macro", "test_pp_parse_macro_first_line");
	assert_equal_string(macro->result, "c", "test_pp_parse_macro_first_line");
	assert_equal_string(macro->operand1, "a", "test_pp_parse_macro_first_line");
	assert_equal_string(macro->operand2, "b", "test_pp_parse_macro_first_line");
	assert_true(macro->is_binary, "Macro should be binary", "test_pp_parse_macro_first_line");

	// C, A and B should all be in the temporary variables set
	assert_equal_int(p2.variables->size(), 3, "test_pp_parse_macro_first_line");
	assert_equal_int(p2.variables->count("c"), 1, "test_pp_parse_macro_first_line");
	assert_equal_int(p2.variables->count("a"), 1, "test_pp_parse_macro_first_line");
	assert_equal_int(p2.variables->count("b"), 1, "test_pp_parse_macro_first_line");

	// only the dummy operands A and B should be in the temporary defined variables set
	assert_equal_int(p2.defined_variables->size(), 2, "test_pp_parse_macro_first_line");
	assert_equal_int(p2.defined_variables->count("a"), 1, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p2.defined_variables->count("b"), 1, "test_pp_parse_macro_subsequent_line");


	pass("test_pp_parse_macro_first_line");

}


void test_pp_parse_macro_subsequent_line() {

	Preprocessor p;
	struct macro *macro = new struct macro();
	p.parse_macro_first_line("#macro c = my_macro a b", macro);

	// trivial errors
	assert_equal_int(p.parse_macro_subsequent_line("declare intvar q", NULL), OTHER_ERROR, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.parse_macro_subsequent_line("", macro), 0, "test_pp_parse_macro_subsequent_line");

	// simple errors
	assert_equal_int(p.parse_macro_subsequent_line("declare q", macro), INVALID_LINE, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.parse_macro_subsequent_line("define q = 3", macro), INVALID_LINE, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.parse_macro_subsequent_line("declare output q", macro), BAD_VAR_TYPE, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.parse_macro_subsequent_line("define a = 3", macro), INVALID_LINE, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.parse_macro_subsequent_line("declare intvar c", macro), INVALID_LINE, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.parse_macro_subsequent_line("declare intvar a", macro), INVALID_LINE, "test_pp_parse_macro_subsequent_line");

	// first a successful declaration
	// then some more errors, a successful definition, and an unsuccessful re-definition
	assert_equal_int(p.parse_macro_subsequent_line("declare intvar q", macro), 0, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.parse_macro_subsequent_line("define p = mul q a", macro), INVALID_LINE, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.parse_macro_subsequent_line("define q = mul q a", macro), INVALID_LINE, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.parse_macro_subsequent_line("define q = mul b a", macro), 0, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.parse_macro_subsequent_line("define q = ln a", macro), INVALID_LINE, "test_pp_parse_macro_subsequent_line");

	// another declaration and definition, and an unsuccessful attempted redefinition
	assert_equal_int(p.parse_macro_subsequent_line("declare intvar p", macro), 0, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.parse_macro_subsequent_line("define p = ln q", macro), 0, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.parse_macro_subsequent_line("define q = ln p", macro), INVALID_LINE, "test_pp_parse_macro_subsequent_line");

	// some unsuccessful definitions of c, then a proper one
	assert_equal_int(p.parse_macro_subsequent_line("define c = pow q x", macro), INVALID_LINE, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.parse_macro_subsequent_line("define c = ln c", macro), INVALID_LINE, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.parse_macro_subsequent_line("define c = exp p", macro), 0, "test_pp_parse_macro_subsequent_line");

	// C, A, B, P and Q should all be in the variables set
	assert_equal_int(p.variables->size(), 5, "test_pp_parse_macro_subsequent_line");
	assert_true(p.variables->at("c") == VariableType::INTVAR, "Intermediate variable should be intvar", "test_pp_parse_macro_subsequent_line");
	assert_true(p.variables->at("a") == VariableType::INTVAR, "Intermediate variable should be intvar", "test_pp_parse_macro_subsequent_line");
	assert_true(p.variables->at("b") == VariableType::INTVAR, "Intermediate variable should be intvar", "test_pp_parse_macro_subsequent_line");
	assert_true(p.variables->at("q") == VariableType::INTVAR, "Intermediate variable should be intvar", "test_pp_parse_macro_subsequent_line");
	assert_true(p.variables->at("p") == VariableType::INTVAR, "Intermediate variable should be intvar", "test_pp_parse_macro_subsequent_line");

	// C, A, B, P and Q should all be in the defined variables set
	assert_equal_int(p.defined_variables->size(), 5, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.defined_variables->count("c"), 1, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.defined_variables->count("a"), 1, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.defined_variables->count("b"), 1, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.defined_variables->count("q"), 1, "test_pp_parse_macro_subsequent_line");
	assert_equal_int(p.defined_variables->count("p"), 1, "test_pp_parse_macro_subsequent_line");

	pass("test_pp_parse_macro_subsequent_line");

}

void test_pp_is_valid_declare_line() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// simple errors
	assert_equal_int(p.is_valid_declare_line(""), OTHER_ERROR, "test_pp_is_valid_declare_line");
	assert_equal_int(p.is_valid_declare_line("declare input"), INVALID_LINE, "test_pp_is_valid_declare_line");
	assert_equal_int(p.is_valid_declare_line("declare x"), INVALID_LINE, "test_pp_is_valid_declare_line");
	assert_equal_int(p.is_valid_declare_line("declare vector input x"), INVALID_LINE, "test_pp_is_valid_declare_line");
	assert_equal_int(p.is_valid_declare_line("eclare input x"), INVALID_LINE, "test_pp_is_valid_declare_line");
	assert_equal_int(p.is_valid_declare_line("declare inpu x"), BAD_VAR_TYPE, "test_pp_is_valid_declare_line");
	assert_equal_int(p.is_valid_declare_line("declare input add"), INVALID_VAR_NAME, "test_pp_is_valid_declare_line");
	assert_equal_int(p.is_valid_declare_line("declare input 3"), INVALID_VAR_NAME, "test_pp_is_valid_declare_line");

	// successful declare then unsuccessful redeclaring
	assert_equal_int(p.is_valid_declare_line("declare input x"), 0, "test_pp_is_valid_declare_line");
	p.expand_line("declare input x", write_scratch_file);
	assert_equal_int(p.is_valid_declare_line("declare input x"), VAR_DECLARED_TWICE, "test_pp_is_valid_declare_line");
	assert_equal_int(p.is_valid_declare_line("declare weight x"), VAR_DECLARED_TWICE, "test_pp_is_valid_declare_line");

	write_scratch_file.close();
	pass("test_pp_is_valid_declare_line");

}

void test_pp_is_valid_define_line() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// define two macros which will be used later in this test
	p.expand_line("#macro c = my_binary_macro a b; declare intvar p; define p = pow a b; define c = ln p", write_scratch_file);
	p.expand_line("#macro z = my_unary_macro x; declare intvar q; define z = logistic x", write_scratch_file);

	// simple errors
	assert_equal_int(p.is_valid_define_line(""), OTHER_ERROR, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("declare weight x"), INVALID_LINE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define x 3"), INVALID_LINE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define x = add a b c"), INVALID_LINE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("defin x = ln y"), INVALID_LINE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define add = 9"), INVALID_VAR_NAME, "test_pp_is_valid_define_line");
	
	// cannot define input, weight or exp_output
	p.expand_line("declare input x", write_scratch_file);
	p.expand_line("declare weight w", write_scratch_file);
	p.expand_line("declare exp_output y", write_scratch_file);
	assert_equal_int(p.is_valid_define_line("define x = 9"), CANNOT_DEFINE_I_W_EO, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define w = mul x x"), CANNOT_DEFINE_I_W_EO, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define y = exp w"), CANNOT_DEFINE_I_W_EO, "test_pp_is_valid_define_line");

	// cannot define variables that are not declared
	assert_equal_int(p.is_valid_define_line("define a = exp w"), VAR_DEFINED_BEFORE_DECLARED, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define b = -1"), VAR_DEFINED_BEFORE_DECLARED, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define c = my_macro x w"), VAR_DEFINED_BEFORE_DECLARED, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define d = x"), VAR_DEFINED_BEFORE_DECLARED, "test_pp_is_valid_define_line");

	// cannot define variables that have already been defined
	p.expand_line("declare intvar z", write_scratch_file);
	p.expand_line("define z = mul x w", write_scratch_file);
	assert_equal_int(p.is_valid_define_line("define z = exp w"), VAR_DEFINED_TWICE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define z = -1"), VAR_DEFINED_TWICE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define z = my_macro x w"), VAR_DEFINED_TWICE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define z = x"), VAR_DEFINED_TWICE, "test_pp_is_valid_define_line");

	// successful definition as a constant
	p.expand_line("declare intvar p", write_scratch_file);
	assert_equal_int(p.is_valid_define_line("define p = 3"), 0, "test_pp_is_valid_define_line");
	p.expand_line("declare intvar q", write_scratch_file);

	// some errors with binary primitive definitions
	assert_equal_int(p.is_valid_define_line("define p = add x"), INVALID_LINE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = pow x q"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = mul q 3"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_line");

	// some successful binary primitive definitions
	assert_equal_int(p.is_valid_define_line("define p = add x z"), 0, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = mul z -23"), 0, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = pow w w"), 0, "test_pp_is_valid_define_line");

	// some errors with unary primitive definitions
	assert_equal_int(p.is_valid_define_line("define p = ln x y"), INVALID_LINE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = logistic q"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = exp p"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_line");

	// some successful unary primitive definitions
	assert_equal_int(p.is_valid_define_line("define p = exp x"), 0, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = logistic -23"), 0, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = ln 0"), 0, "test_pp_is_valid_define_line");

	// some errors with definitions as equivalent to other variables
	assert_equal_int(p.is_valid_define_line("define p = p"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = q"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = n"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_line");

	// some successful definitions as equivalent to another variable
	assert_equal_int(p.is_valid_define_line("define p = x"), 0, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = w"), 0, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = z"), 0, "test_pp_is_valid_define_line");

	p.expand_line("declare_vector intvar u 3", write_scratch_file);
	p.expand_line("declare_vector input v 3", write_scratch_file);
	p.expand_line("declare_vector input four_d_vec 4", write_scratch_file);
	p.expand_line("declare_vector output t 3", write_scratch_file);
	p.expand_line("declare_vector intvar f 4", write_scratch_file);

	// some general errors with vector operation definitions
	assert_equal_int(p.is_valid_define_line("define n = dot u v"), VAR_DEFINED_BEFORE_DECLARED, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define z = dot u v"), VAR_DEFINED_TWICE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define t = dot u u"), VAR_DEFINED_BEFORE_DECLARED, "test_pp_is_valid_define_line");

	// some errors with binary vector operation definitions
	assert_equal_int(p.is_valid_define_line("define p = dot u v t"), INVALID_LINE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = dot v"), INVALID_LINE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = dot u v"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = dot v four_d_vec"), VECTORS_OF_DIFFERENT_DIMENSION, "test_pp_is_valid_define_line");

	// some successful binary vector operation definitions
	assert_equal_int(p.is_valid_define_line("define p = dot v v"), 0, "test_pp_is_valid_define_line");

	p.expand_line("define_vector u = mul v 2", write_scratch_file);
	assert_equal_int(p.is_valid_define_line("define q = dot u v"), 0, "test_pp_is_valid_define_line");


	// some general errors with using a macro in a definition
	assert_equal_int(p.is_valid_define_line("define p = non_existent_macro x w"), INVALID_LINE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define foo = my_binary_macro x w"), VAR_DEFINED_BEFORE_DECLARED, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define z = my_unary_macro x"), VAR_DEFINED_TWICE, "test_pp_is_valid_define_line");
	
	
	// some errors with using a binary macro in a definition
	assert_equal_int(p.is_valid_define_line("define p = my_binary_macro x"), INVALID_LINE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = my_binary_macro p x"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = my_binary_macro 3 foo"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_line");

	// some errors with using a unary macro in a defintion
	assert_equal_int(p.is_valid_define_line("define p = my_unary_macro x w"), INVALID_LINE, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = my_unary_macro p"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_line");

	// some successful binary macro definitions
	assert_equal_int(p.is_valid_define_line("define p = my_binary_macro w x"), 0, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = my_binary_macro 3 x"), 0, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = my_binary_macro z 0"), 0, "test_pp_is_valid_define_line");


	// some successful unary macro definitions
	assert_equal_int(p.is_valid_define_line("define p = my_unary_macro w"), 0, "test_pp_is_valid_define_line");
	assert_equal_int(p.is_valid_define_line("define p = my_unary_macro -0.23"), 0, "test_pp_is_valid_define_line");


	write_scratch_file.close();
	pass("test_pp_is_valid_define_line");

}

void test_pp_is_valid_define_vector_line() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// define two macros which will be used later in this test
	p.expand_line("#macro c = my_binary_macro a b; declare intvar p; define p = pow a b; define c = ln p", write_scratch_file);
	p.expand_line("#macro z = my_unary_macro x; declare intvar q; define z = logistic x", write_scratch_file);

	// simple errors
	assert_equal_int(p.is_valid_define_vector_line(""), OTHER_ERROR, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("declare weight x"), INVALID_LINE, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector x y"), INVALID_LINE, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector x = add a b c"), INVALID_LINE, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("defin_vector x = ln y"), INVALID_LINE, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector add = add x y"), INVALID_VAR_NAME, "test_pp_is_valid_define_vector_line");
	
	// cannot define input, weight or exp_output vectors
	p.expand_line("declare_vector input x 3", write_scratch_file);
	p.expand_line("declare_vector weight w 3", write_scratch_file);
	p.expand_line("declare_vector exp_output y 5", write_scratch_file);
	assert_equal_int(p.is_valid_define_vector_line("define_vector x = add y 3"), CANNOT_DEFINE_I_W_EO, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector w = mul x x"), CANNOT_DEFINE_I_W_EO, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector y = my_unary_macro w"), CANNOT_DEFINE_I_W_EO, "test_pp_is_valid_define_vector_line");

	// cannot define vectors that are not declared
	assert_equal_int(p.is_valid_define_vector_line("define_vector a = exp w"), VAR_DEFINED_BEFORE_DECLARED, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector b = add x x"), VAR_DEFINED_BEFORE_DECLARED, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector c = my_binary_macro x w"), VAR_DEFINED_BEFORE_DECLARED, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector d = logistic x"), VAR_DEFINED_BEFORE_DECLARED, "test_pp_is_valid_define_vector_line");

	// cannot define vectors that have already been defined
	assert_equal_int(p.expand_line("declare_vector intvar z 3", write_scratch_file), 3, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.expand_line("define_vector z = mul x w", write_scratch_file), 3, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector z = exp w"), VAR_DEFINED_TWICE, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector z = add x w"), VAR_DEFINED_TWICE, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector z = my_binary_macro x w"), VAR_DEFINED_TWICE, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector z = ln x"), VAR_DEFINED_TWICE, "test_pp_is_valid_define_vector_line");

	// declare intvar vectors P and Q
	// declare input M and intvar vectors A and B
	p.expand_line("declare_vector intvar p 3", write_scratch_file);
	p.expand_line("declare_vector intvar q 4", write_scratch_file);
	p.expand_line("declare input m", write_scratch_file);
	p.expand_line("declare_vector intvar a 3", write_scratch_file);

	// some simple errors with binary primitive/macro definitions
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = add x"), INVALID_LINE, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = pow m 2"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = my_binary_macro a m"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector q = my_binary_macro x m"), VECTORS_OF_DIFFERENT_DIMENSION, "test_pp_is_valid_define_vector_line");

	// some errors with binary primitive/macro operations on two vectors
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = mul x a"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = my_binary_macro x y"), VECTORS_OF_DIFFERENT_DIMENSION, "test_pp_is_valid_define_vector_line");

	// some errors with binary primitive/macro operations on a vector and scalar variable
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = pow x foo"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_vector_line");
	p.expand_line("declare intvar bar", write_scratch_file);
	assert_equal_int(p.is_valid_define_vector_line("define_vector a = add x bar"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_vector_line");
	
	// some successful vector definitions as binary primitive/macro operations
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = mul x w"), 0, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = my_binary_macro w x"), 0, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector a = add x m"), 0, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = my_binary_macro w m"), 0, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = pow x 2"), 0, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector a = my_binary_macro w -121.2"), 0, "test_pp_is_valid_define_vector_line");


	// some errors with unary primitive/macro definitions
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = ln x w"), INVALID_LINE, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = logistic m"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = my_unary_macro a"), VAR_REFERENCED_BEFORE_DEFINED, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector q = exp x"), VECTORS_OF_DIFFERENT_DIMENSION, "test_pp_is_valid_define_vector_line");

	// some successful vector definitions as unary primitive/macro operations
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = exp x"), 0, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector p = logistic w"), 0, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector a = ln x"), 0, "test_pp_is_valid_define_vector_line");
	assert_equal_int(p.is_valid_define_vector_line("define_vector a = my_unary_macro w"), 0, "test_pp_is_valid_define_vector_line");

	pass("test_pp_is_valid_define_vector_line");

}

void test_pp_is_valid_declare_vector_line() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// simple errors
	assert_equal_int(p.is_valid_declare_vector_line(""), OTHER_ERROR, "test_pp_is_valid_declare_vector_line");
	assert_equal_int(p.is_valid_declare_vector_line("declare_vector input vec"), INVALID_LINE, "test_pp_is_valid_declare_vector_line");
	assert_equal_int(p.is_valid_declare_vector_line("declare_vector vec 3"), INVALID_LINE, "test_pp_is_valid_declare_vector_line");
	assert_equal_int(p.is_valid_declare_vector_line("declare vector input vec 3"), INVALID_LINE, "test_pp_is_valid_declare_vector_line");
	assert_equal_int(p.is_valid_declare_vector_line("eclare_vector input vec 3"), INVALID_LINE, "test_pp_is_valid_declare_vector_line");
	assert_equal_int(p.is_valid_declare_vector_line("declare_vector inpu vec 3"), BAD_VAR_TYPE, "test_pp_is_valid_declare_vector_line");
	assert_equal_int(p.is_valid_declare_vector_line("declare_vector input mul 3"), INVALID_VAR_NAME, "test_pp_is_valid_declare_vector_line");
	assert_equal_int(p.is_valid_declare_vector_line("declare input 3"), INVALID_LINE, "test_pp_is_valid_declare_vector_line");
	assert_equal_int(p.is_valid_declare_vector_line("declare_vector input vec 3.2"), BAD_VECTOR_SIZE, "test_pp_is_valid_declare_vector_line");
	assert_equal_int(p.is_valid_declare_vector_line("declare_vector input vec -2"), BAD_VECTOR_SIZE, "test_pp_is_valid_declare_vector_line");
	assert_equal_int(p.is_valid_declare_vector_line("declare_vector input vec 0"), BAD_VECTOR_SIZE, "test_pp_is_valid_declare_vector_line");
	assert_equal_int(p.is_valid_declare_vector_line("declare_vector input vec " + to_string(MAX_VECTOR_SIZE + 1)), BAD_VECTOR_SIZE, "test_pp_is_valid_declare_vector_line");

	// successful declare then unsuccessful redeclaring
	assert_equal_int(p.is_valid_declare_vector_line("declare_vector input vec 3"), 0, "test_pp_is_valid_declare_vector_line");
	p.expand_line("declare_vector input vec 3", write_scratch_file);
	assert_equal_int(p.is_valid_declare_vector_line("declare_vector input vec 4"), VAR_DECLARED_TWICE, "test_pp_is_valid_declare_vector_line");
	assert_equal_int(p.is_valid_declare_vector_line("declare_vector weight vec 3"), VAR_DECLARED_TWICE, "test_pp_is_valid_declare_vector_line");

	write_scratch_file.close();
	pass("test_pp_is_valid_declare_vector_line");
}

void test_vector_component_functions() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// declare vector x
	assert_equal_int(p.expand_line("declare_vector intvar x 3", write_scratch_file), 3, "test_vector_component_functions");
	// make sure x's components are marked as defined
	// check basic error catching of is_vector_component
	assert_true(p.is_vector_component("x.0"), "X.0 should be a valid component", "test_vector_component_functions");
	assert_false(p.is_vector_component("x.3"), "X.2 is a valid component", "test_vector_component_functions");
	assert_false(p.is_vector_component("x1"), "X1 is NOT a valid component", "test_vector_component_functions");
	assert_false(p.is_vector_component("y.0"), "Y.0 is a valid component", "test_vector_component_functions");

	// check that none of X's components have been defined
	assert_false(p.has_defined_components("y"), "Y has no defined components", "test_vector_component_functions");
	assert_false(p.has_defined_components("x"), "X has no defined components yet", "test_vector_component_functions");
	// define x.0 individually, make sure x now has defined components
	assert_equal_int(p.expand_line("define x.0 = sub 3 2", write_scratch_file), 1, "test_vector_component_functions");
	assert_true(p.has_defined_components("x"), "X now has defined components", "test_vector_component_functions");

	// check that not all of X's components are defined
	assert_false(p.all_components_defined("x"), "Not all of X's components are defined yet", "test_vector_component_functions");
	assert_false(p.all_components_defined("Y"), "Not all of Y's components are defined (Y is not defined at all)", "test_vector_component_functions");
	// define x.1 and x.2
	assert_equal_int(p.expand_line("define x.1 = add x.0 1", write_scratch_file), 1, "test_vector_component_functions");
	assert_equal_int(p.expand_line("define x.2 = 3", write_scratch_file), 1, "test_vector_component_functions");
	// check that all of X's components are defined
	assert_true(p.all_components_defined("x"), "All of X's components are defined", "test_vector_component_functions");

	pass("test_vector_component_functions");

}

void test_define_vector_components() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// declare an intvar and input vector
	// make sure all the components of both vectors are in the variables map
	// make sure the components of the input vector are in the defined_variables set
	assert_equal_int(p.expand_line("declare_vector input x 3", write_scratch_file), 3, "test_define_vector_components");
	assert_equal_int(p.expand_line("declare_vector intvar z 3", write_scratch_file), 3, "test_define_vector_components");

	assert_equal_int(p.variables->size(), 6, "test_define_vector_components");
	assert_equal_int(p.vectors->size(), 2, "test_define_vector_components");
	assert_equal_int(p.vectors->count("x"), 1, "test_define_vector_components");
	assert_equal_int(p.vectors->count("z"), 1, "test_define_vector_components");

	assert_equal_int(p.variables->count("x.0"), 1, "test_define_vector_components");
	assert_equal_int(p.variables->count("x.1"), 1, "test_define_vector_components");
	assert_equal_int(p.variables->count("x.2"), 1, "test_define_vector_components");
	assert_equal_int(p.variables->count("z.0"), 1, "test_define_vector_components");
	assert_equal_int(p.variables->count("z.1"), 1, "test_define_vector_components");
	assert_equal_int(p.variables->count("z.2"), 1, "test_define_vector_components");
	
	assert_equal_int(p.defined_variables->size(), 4, "test_define_vector_components");
	assert_equal_int(p.defined_variables->count("x"), 1, "test_define_vector_components");
	assert_equal_int(p.defined_variables->count("x.0"), 1, "test_define_vector_components");
	assert_equal_int(p.defined_variables->count("x.1"), 1, "test_define_vector_components");
	assert_equal_int(p.defined_variables->count("x.2"), 1, "test_define_vector_components");

	// try to define a component of input vector X. This should fail.
	assert_equal_int(p.expand_line("define x.1 = 3", write_scratch_file), CANNOT_DEFINE_I_W_EO, "test_define_vector_components");

	// define Z.0. This should succeed.
	// Z.0 should now be in the defined_variables set
	assert_equal_int(p.expand_line("define z.0 = 10", write_scratch_file), 1, "test_define_vector_components");
	assert_equal_int(p.defined_variables->count("z.0"), 1, "test_define_vector_components");

	// try to redefine Z.0. This should fail.
	assert_equal_int(p.expand_line("define z.0 = 0", write_scratch_file), VAR_DEFINED_TWICE, "test_define_vector_components");

	// declare intvar vector P.
	assert_equal_int(p.expand_line("declare_vector intvar p 3", write_scratch_file), 3, "test_define_vector_components");
	// define P as an operation of X and Z.
	// This should fail because Z is not fully defined
	assert_equal_int(p.expand_line("define_vector p = mul x z", write_scratch_file), VAR_REFERENCED_BEFORE_DEFINED, "test_define_vector_components");

	// try to define_vector Z as an operation of X.
	// this should fail because one of Z's components has already been defined
	assert_equal_int(p.expand_line("define_vector z = exp x", write_scratch_file), VAR_DEFINED_TWICE, "test_define_vector_components");

	// define Z.1 and Z.2
	// these should succeed.  Make sure Z.1 and Z.2 are now both in the defined_variables set
	assert_equal_int(p.expand_line("define z.1 = 20", write_scratch_file), 1, "test_define_vector_components");
	assert_equal_int(p.expand_line("define z.2 = 30", write_scratch_file), 1, "test_define_vector_components");
	assert_equal_int(p.defined_variables->count("z.1"), 1, "test_define_vector_components");
	assert_equal_int(p.defined_variables->count("z.2"), 1, "test_define_vector_components");

	// define P as a function of Z
	// this should succeed
	assert_equal_int(p.expand_line("define_vector p = add z z", write_scratch_file), 3, "test_define_vector_components");
	// P and its components should now all be in the defined_variables set
	assert_equal_int(p.defined_variables->count("p"), 1, "test_define_vector_components");
	assert_equal_int(p.defined_variables->count("p.0"), 1, "test_define_vector_components");
	assert_equal_int(p.defined_variables->count("p.1"), 1, "test_define_vector_components");
	assert_equal_int(p.defined_variables->count("p.2"), 1, "test_define_vector_components");
	// Z itself should now be in the defined_variables set
	//	(During P's definition, we should realize all of Z's components are defined, and thus mark Z as defined)
	assert_equal_int(p.defined_variables->count("z"), 1, "test_define_vector_components");

	// try to define p.0. This should fail because P (and thus P.0) is already defined
	assert_equal_int(p.expand_line("define p.0 = 2", write_scratch_file), VAR_DEFINED_TWICE, "test_define_vector_components");

	pass("test_define_vector_components");

}

void test_is_valid_reduce_vector_line() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// define a binary macro
	assert_equal_int(p.expand_line("#macro c = my_binary_macro a b; declare intvar p; define p = mul b a; define c = logistic p;", write_scratch_file), 0, "test_pp_expand_line");

	// declare an input vector X, input foo, and intvar Z
	assert_equal_int(p.expand_line("declare_vector input x 3", write_scratch_file), 3, "test_pp_expand_define_vector");
	assert_equal_int(p.expand_line("declare intvar z", write_scratch_file), 1, "test_pp_expand_define_vector");
	assert_equal_int(p.expand_line("declare input foo", write_scratch_file), 1, "test_pp_expand_define_vector");

	// declare intvar vector V
	assert_equal_int(p.expand_line("declare_vector intvar v 3", write_scratch_file), 3, "test_pp_expand_define_vector");

	// trivial errors
	assert_equal_int(p.is_valid_define_line("define z = reduce_vector x"), INVALID_LINE, "test_is_valid_reduce_vector_line");
	assert_equal_int(p.is_valid_define_line("define p = reduce_vector x add"), VAR_DEFINED_BEFORE_DECLARED, "test_is_valid_reduce_vector_line");
	assert_equal_int(p.is_valid_define_line("define foo = reduce_vector x add"), CANNOT_DEFINE_I_W_EO, "test_is_valid_reduce_vector_line");
	assert_equal_int(p.is_valid_define_line("define z = reduce_vector p mul"), VAR_REFERENCED_BEFORE_DEFINED, "test_is_valid_reduce_vector_line");
	assert_equal_int(p.is_valid_define_line("define z = reduce_vector v add"), VAR_REFERENCED_BEFORE_DEFINED, "test_is_valid_reduce_vector_line");
	assert_equal_int(p.is_valid_define_line("define z = reduce_vector x addition"), INVALID_LINE, "test_is_valid_reduce_vector_line");
	assert_equal_int(p.is_valid_define_line("define z = reduce_vector x non_existent_macro"), INVALID_LINE, "test_is_valid_reduce_vector_line");

	// successful definition of Z as a reduction of X
	assert_equal_int(p.is_valid_define_line("define z = reduce_vector x mul"), 0, "test_is_valid_reduce_vector_line");
	// define V's components, make sure the reduction of V succeeds
	assert_equal_int(p.expand_line("define v.0 = 3", write_scratch_file), 1, "test_pp_expand_define_vector");
	assert_equal_int(p.expand_line("define v.1 = 4", write_scratch_file), 1, "test_pp_expand_define_vector");
	assert_equal_int(p.expand_line("define v.2 = 5", write_scratch_file), 1, "test_pp_expand_define_vector");
	assert_equal_int(p.is_valid_define_line("define z = reduce_vector v my_binary_macro"), 0, "test_is_valid_reduce_vector_line");
	// make sure V is now marked as defined
	assert_equal_int(p.defined_variables->count("v"), 1, "test_is_valid_reduce_vector_line");

	pass("test_is_valid_reduce_vector_line");

}

void test_reduce_vector() {

	Preprocessor p;
	ofstream write_scratch_file("scratch.tf");

	// define a binary macro
	assert_equal_int(p.expand_line("#macro c = my_binary_macro a b; declare intvar p; define p = mul b a; define c = logistic p;", write_scratch_file), 0, "test_pp_expand_line");

	// declare input vector X
	assert_equal_int(p.expand_line("declare_vector input x 3", write_scratch_file), 3, "test_reduce_vector");

	// define a as the sum of the components in X (using expand_line)
	assert_equal_int(p.expand_line("declare intvar a", write_scratch_file), 1, "test_reduce_vector");
	assert_equal_int(p.expand_line("define a = reduce_vector x add", write_scratch_file), 5, "test_reduce_vector");
	string a_reduction_lines[5] = {"declare intvar a.0", "define a.0 = add x.0 x.1",
		"declare intvar a.1", "define a.1 = add a.0 x.2", "define a = a.1"};
	assert_equal_file_lines("scratch.tf", a_reduction_lines, 4, 5, "test_reduce_vector");

	// define B as the product of the components in X
	assert_equal_int(p.expand_line("declare intvar b", write_scratch_file), 1, "test_reduce_vector");
	assert_equal_int(p.expand_reduce_vector_instruction("b", "x", "mul", 3, write_scratch_file), 5, "test_reduce_vector");
	string b_reduction_lines[5] = {"declare intvar b.0", "define b.0 = mul x.0 x.1",
		"declare intvar b.1", "define b.1 = mul b.0 x.2", "define b = b.1"};
	assert_equal_file_lines("scratch.tf", b_reduction_lines, 10, 5, "test_reduce_vector");

	// define C as the reduction of X under the function my_binary_macro
	assert_equal_int(p.expand_line("declare intvar c", write_scratch_file), 1, "test_reduce_vector");
	assert_equal_int(p.expand_reduce_vector_instruction("c", "x", "my_binary_macro", 3, write_scratch_file), 9, "test_reduce_vector");
	string c_reduction_lines[9] = {
		"declare intvar c.0", "declare intvar c.0_p0", "define c.0_p0 = mul x.1 x.0", "define c.0 = logistic c.0_p0",
		"declare intvar c.1", "declare intvar c.1_p1", "define c.1_p1 = mul x.2 c.0", "define c.1 = logistic c.1_p1",
		"define c = c.1"
	};
	assert_equal_file_lines("scratch.tf", c_reduction_lines, 16, 9, "test_reduce_vector");

	// edge case, define D as the reduction of Y, a one-element vector under mul
	assert_equal_int(p.expand_line("declare intvar d", write_scratch_file), 1, "test_reduce_vector");
	assert_equal_int(p.expand_line("declare_vector weight y 1", write_scratch_file), 1, "test_reduce_vector");
	assert_equal_int(p.expand_reduce_vector_instruction("d", "y", "mul", 1, write_scratch_file), 1, "test_reduce_vector");
	string d_reduction_lines[1] = {"define d = y.0"};
	assert_equal_file_lines("scratch.tf", d_reduction_lines, 27, 1, "test_reduce_vector");

	// edge case, define E as the reduction of Y, a one-element vector, under my_binary_macro
	assert_equal_int(p.expand_line("declare intvar e", write_scratch_file), 1, "test_reduce_vector");
	assert_equal_int(p.expand_reduce_vector_instruction("e", "y", "my_binary_macro", 1, write_scratch_file), 1, "test_reduce_vector");
	string e_reduction_lines[1] = {"define e = y.0"};
	assert_equal_file_lines("scratch.tf", e_reduction_lines, 29, 1, "test_reduce_vector");

	pass("test_reduce_vector");
}


void run_pp_tests() {

	cout << "\nTesting Preprocessor Class... " << endl << endl;

	test_pp_constructor_expand_destructor();
	test_pp_expand_line();
	test_pp_expand_declare_vector();
	test_pp_expand_define();
	test_pp_expand_define_vector();
	test_pp_expand_vector_operation();
	test_pp_expand_unary_macro();
	test_pp_expand_binary_macro();
	test_pp_substitute_dummy_names();
	test_pp_macro_validation();
	test_pp_expand_dot_product();
	test_pp_expand_vector_add();
	test_pp_expand_vector_mul();
	test_pp_expand_vector_scale();
	test_pp_expand_vector_increment();
	test_pp_parse_macro_line();
	test_pp_parse_macro_first_line();
	test_pp_parse_macro_subsequent_line();
	test_pp_is_valid_declare_line();
	test_pp_is_valid_define_line();
	test_pp_is_valid_define_vector_line();
	test_pp_is_valid_declare_vector_line();
	test_vector_component_functions();
	test_define_vector_components();
	test_is_valid_reduce_vector_line();
	test_reduce_vector();

	cout << "\nAll Preprocessor Tests Passed." << endl << endl;

}