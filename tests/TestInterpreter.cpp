#include <iostream>
#include <fstream>
#include <cfloat>

#include "TestInterpreter.h"
#include "../src/Interpreter.h"
#include "TestUtilities.h"

using namespace std;



void test_interp_constructor_interpret_destructor() {

	// construct a new Interpreter
	// interpret a new program, and make sure this works successfully
	// make sure the Interpreter is deleted without problems
	Interpreter *i = new Interpreter();
	unordered_map<string, double> inputs;
	unordered_map<string, double> *outputs = new unordered_map<string, double>();
	
	inputs["a.0"] = 1; inputs["a.1"] = 2; inputs["a.2"] = 1;
	inputs["b.0"] = 1; inputs["b.1"] = 2; inputs["b.2"] = -1;
	inputs["c.0"] = 2; inputs["c.1"] = 4; inputs["c.2"] = 6;
	inputs["d.0"] = 1; inputs["d.1"] = 3; inputs["d.2"] =  5;
	inputs["e.0"] = 1; inputs["e.1"] = 2; inputs["e.2"] = 3;
	inputs["f.0"] = 2; inputs["f.1"] = 3; inputs["f.2"] = 4;

	string exp_output_vars[23] = {
		"foo", "bar", "baz", "baz_squared",
		"A.0", "A.1", "A.2", "B.0", "B.1", "B.2",
		"C.0", "C.1", "C.2", "D.0", "D.1", "D.2",
		"E.0", "E.1", "E.2", "F.0", "F.1", "F.2",
		"G"
	};

	double exp_output_values[23] = {
		4, 54.59815, 4, 16,
		3, 7, 11, 2, 12, 36,
		12, 28, 44, 4, 144, 1296,
		140, 780, 1932, (double) logistic(4), (double) logistic(144), (double) logistic(1296),
		(double) (140*logistic(4) + 780*logistic(144) + 1932*logistic(1296))
	};
	
	assert_equal_int(i->interpret("tests/test_files/inputs/expanded_shape_simple.tf", inputs, outputs), 0, "test_interp_constructor_destructor");

	for (int i = 0; i < 23; i++) {
		assert_equal_double(outputs->at(exp_output_vars[i]), exp_output_values[i], "test_interp_constructor_destructor");
	} 
	
	delete i;
	pass("test_interp_constructor_interpret_destructor");

}

void test_interp_parse_input_file() {

	Interpreter i;
	unordered_map<string, double> input_map;

	// parse the input file
	assert_equal_int(i.parse_input_file("tests/test_files/inputs/interpreter_inputs.txt", &input_map), 0, "test_interp_parse_input_file");

	string input_vars[26] = {"a", "b", "c", "d", "e", "f", "g",
		"h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r",
		"s", "t", "u", "v", "w", "x", "y", "z"};
	double input_values[26] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};
	
	// build the expected input map
	unordered_map<string, double> expected_input_map;
	for (int i = 0; i < 26; i++) {
		expected_input_map.insert(make_pair(input_vars[i], input_values[i]));
	}

	// make sure the input map after parsing the input file is as expected
	for (unordered_map<string, double>::iterator it = input_map.begin();
		it != input_map.end(); ++it) {
		string var_name = it->first;
		assert_equal_double(it->second, expected_input_map.at(var_name), "test_interp_parse_input_file");
	}

	pass("test_interp_parse_input_file");

}

void test_interp_parse_input_line() {

	Interpreter i;
	string input_var_name;
	double input_var_value;

	// empty line
	assert_equal_int(i.parse_input_line("", &input_var_name, &input_var_value), 0, "test_interp_parse_input_line");
	assert_equal_string(input_var_name, "", "test_interp_parse_input_line");
	assert_equal_double(input_var_value, 0, "test_interp_parse_input_line");

	// errors
	assert_equal_int(i.parse_input_line("a 1", &input_var_name, &input_var_value), INVALID_LINE, "test_interp_parse_input_line");
	assert_equal_int(i.parse_input_line("a\tone", &input_var_name, &input_var_value), INVALID_LINE, "test_interp_parse_input_line");
	assert_equal_int(i.parse_input_line("a\t1\t2", &input_var_name, &input_var_value), INVALID_LINE, "test_interp_parse_input_line");
	
	assert_equal_int(i.parse_input_line("var_name \t3", &input_var_name, &input_var_value), 0, "test_interp_parse_input_line");
	assert_equal_string(input_var_name, "var_name ", "test_interp_parse_input_line");
	assert_equal_double(input_var_value, 3, "test_interp_parse_input_line");

	pass("test_interp_parse_input_line");

}

void test_interp_accumulate_outputs() {

	Interpreter i;

	// manually add the variables A, B and C
	// give them some values
	// make B and C output values
	BindingsDictionary *bd = i.get_bindings_dictionary();
	unordered_map<string, VariableType> *var_types = i.get_var_types();
	bd->add_variable("a"); bd->add_variable("b"); bd->add_variable("c");
	bd->bind_value("a", 3); bd->bind_value("b", -0.2); bd->bind_value("c", 92.1);
	var_types->insert(make_pair("a", VariableType::INPUT));
	var_types->insert(make_pair("b", VariableType::OUTPUT));
	var_types->insert(make_pair("c", VariableType::OUTPUT));
	
	// build the expected output map
	// it must contain B and C
	unordered_map<string, double> expected_output_map = {{"b", -0.2}, {"c", 92.1}};
	
	// accumulate the outputs, building the output map
	unordered_map<string, double> *output_map = new unordered_map<string, double>();
	i.accumulate_outputs(output_map);

	// make sure the output map after accumulating the outputs is as expected
	for (unordered_map<string, double>::iterator it = output_map->begin();
		it != output_map->end(); ++it) {
		string var_name = it->first;
		assert_equal_double(it->second, expected_output_map.at(var_name), "test_interp_accumulate_outputs");
	}

	pass("test_interp_accumulate_outputs");

}

void test_interp_parse_line() {

	Interpreter i;
	unordered_map<string, double> inputs;
	inputs["x"] = -0.9; inputs["w"] = 3.9; inputs["y"] = -4.1;
	inputs["infinity"] = DBL_MAX; inputs["neg_infinity"] = DBL_MIN;

	// trivial errors/edge cases
	assert_equal_int(i.parse_line("", inputs), 0, "test_interp_parse_line");
	assert_equal_int(i.parse_line("declare y", inputs), INVALID_LINE, "test_interp_parse_line");
	assert_equal_int(i.parse_line("declarate weight w", inputs), INVALID_LINE, "test_interp_parse_line");
	
	// errors with parsing declare instructions
	assert_equal_int(i.parse_line("declare input x x", inputs), INVALID_LINE, "test_interp_parse_line");
	assert_equal_int(i.parse_line("declare expoutput y", inputs), INVALID_LINE, "test_interp_parse_line");
	assert_equal_int(i.parse_line("declare intvar intvar", inputs), INVALID_VAR_NAME, "test_interp_parse_line");

	// successfully declare a variable, then unsuccessfully redeclare it
	assert_equal_int(i.parse_line("declare intvar z", inputs), 0, "test_interp_parse_line");	
	assert_equal_int(i.parse_line("declare output z", inputs), VAR_DECLARED_TWICE, "test_interp_parse_line");

	// make sure variable type is correctly recorded on a declare
	assert_true(i.get_var_types()->at("z") == VariableType::INTVAR, "Z should be an intvar", "test_interp_parse_line");

	// make sure it's an error if the input value is not provided
	assert_equal_int(i.parse_line("declare input foo", inputs), INPUT_VALUE_NOT_PROVIDED, "test_interp_parse_line");

	// make sure the correct declaration of an input/weight/exp_output succeed
	assert_equal_int(i.parse_line("declare input x", inputs), 0, "test_interp_parse_line");
	assert_equal_int(i.parse_line("declare weight w", inputs), 0, "test_interp_parse_line");
	assert_equal_int(i.parse_line("declare exp_output y", inputs), 0, "test_interp_parse_line");

	// make sure the values for X, W and Y are bound, and the var types are correctly recorded
	assert_equal_double(i.get_bindings_dictionary()->get_value("x"), -0.9, "test_interp_parse_line");
	assert_equal_double(i.get_bindings_dictionary()->get_value("w"), 3.9, "test_interp_parse_line");
	assert_equal_double(i.get_bindings_dictionary()->get_value("y"), -4.1, "test_interp_parse_line");
	assert_true(i.get_var_types()->at("x") == VariableType::INPUT, "X should be an input", "test_interp_parse_line");
	assert_true(i.get_var_types()->at("w") == VariableType::WEIGHT, "W should be an weight", "test_interp_parse_line");
	assert_true(i.get_var_types()->at("y") == VariableType::EXP_OUTPUT, "Y should be an exp_output", "test_interp_parse_line");

	// make sure DBL_MIN and DBL_MAX are invalid inputs
	assert_equal_int(i.parse_line("declare input infinity", inputs), OTHER_ERROR, "test_interp_parse_line");
	assert_equal_int(i.parse_line("declare weight neg_infinity", inputs), OTHER_ERROR, "test_interp_parse_line");



	// basic errors with define instructions
	assert_equal_int(i.parse_line("define z 3", inputs), INVALID_LINE, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define define = mul x x", inputs), INVALID_VAR_NAME, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define non_existent_var = add x w", inputs), VAR_DEFINED_BEFORE_DECLARED, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define y = logistic x", inputs), VAR_DEFINED_TWICE, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define z = 3 3", inputs), INVALID_LINE, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define z x", inputs), INVALID_LINE, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define z = add x", inputs), INVALID_LINE, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define z = ln y w", inputs), INVALID_LINE, "test_interp_parse_line");

	// successfully define z as a constant
	assert_equal_int(i.parse_line("define z = -0.2342", inputs), 0, "test_interp_parse_line");
	
	// declare a few more intvars
	assert_equal_int(i.parse_line("declare intvar p", inputs), 0, "test_interp_parse_line");
	assert_equal_int(i.parse_line("declare intvar q", inputs), 0, "test_interp_parse_line");

	// it is an error to define a variable as equivalent to an undefined variable
	assert_equal_int(i.parse_line("define p = non_existent_var", inputs), VAR_REFERENCED_BEFORE_DEFINED, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define q = p", inputs), VAR_REFERENCED_BEFORE_DEFINED, "test_interp_parse_line");

	// successfuly definitions of P and Q as equivalent to other variables
	assert_equal_int(i.parse_line("define p = w", inputs), 0, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define q = p", inputs), 0, "test_interp_parse_line");



	// declare a few more intvars
	assert_equal_int(i.parse_line("declare intvar a", inputs), 0, "test_interp_parse_line");
	assert_equal_int(i.parse_line("declare intvar b", inputs), 0, "test_interp_parse_line");
	assert_equal_int(i.parse_line("declare intvar c", inputs), 0, "test_interp_parse_line");
	assert_equal_int(i.parse_line("declare intvar d", inputs), 0, "test_interp_parse_line");

	// define A using binary operations on two variables
	// first 3 involve undefined variables
	// fourth succeeds
	assert_equal_int(i.parse_line("define a = add foo bar", inputs), VAR_REFERENCED_BEFORE_DEFINED, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define a = mul x foo", inputs), VAR_REFERENCED_BEFORE_DEFINED, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define a = add foo z", inputs), VAR_REFERENCED_BEFORE_DEFINED, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define a = pow w q", inputs), 0, "test_interp_parse_line");

	// define B and C using binary operations on one variable and one constant
	// first 2 involve undefined variables
	// third and fourth fourth should succeed
	assert_equal_int(i.parse_line("define b = add foo 3", inputs), VAR_REFERENCED_BEFORE_DEFINED, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define b = mul 7.9 foo", inputs), VAR_REFERENCED_BEFORE_DEFINED, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define b = pow q 2", inputs), 0, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define c = mul -0.23 p", inputs), 0, "test_interp_parse_line");

	// define D using binary operations on two constants
	// should succeed
	assert_equal_int(i.parse_line("define d = sub 10 0.8", inputs), 0, "test_interp_parse_line");


	// declare a few more intvars
	assert_equal_int(i.parse_line("declare intvar e", inputs), 0, "test_interp_parse_line");
	assert_equal_int(i.parse_line("declare output f", inputs), 0, "test_interp_parse_line");

	// define E using a unary operation on a variable
	// first involves an undefined variable
	// second should succeed
	assert_equal_int(i.parse_line("define e = logistic foo", inputs), VAR_REFERENCED_BEFORE_DEFINED, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define e = ln d", inputs), 0, "test_interp_parse_line");

	// define F using a unary operation on a constant
	// should succeed
	assert_equal_int(i.parse_line("define f = exp e", inputs), 0, "test_interp_parse_line");

	// divide by 0, ln(non-positive) and pow(negative, non-int) should fail
	assert_equal_int(i.parse_line("declare intvar g", inputs), 0, "test_interp_parse_line");

	assert_equal_int(i.parse_line("define g = pow 0 -3", inputs), -1, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define g = ln 0", inputs), -1, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define g = ln -2342.9", inputs), -1, "test_interp_parse_line");
	assert_equal_int(i.parse_line("define g = pow -23.1 2.3", inputs), -1, "test_interp_parse_line");

	// make sure the Bindings Dictionary and VarTypes maps are correct
	assert_equal_double(i.get_bindings_dictionary()->get_value("x"), -0.9, "test_interp_parse_line");
	assert_equal_double(i.get_bindings_dictionary()->get_value("w"), 3.9, "test_interp_parse_line");
	assert_equal_double(i.get_bindings_dictionary()->get_value("y"), -4.1, "test_interp_parse_line");
	assert_equal_double(i.get_bindings_dictionary()->get_value("z"), -0.2342, "test_interp_parse_line");
	assert_equal_double(i.get_bindings_dictionary()->get_value("p"), 3.9, "test_interp_parse_line");
	assert_equal_double(i.get_bindings_dictionary()->get_value("q"), 3.9, "test_interp_parse_line");
	assert_equal_double(i.get_bindings_dictionary()->get_value("a"), pow(3.9, 3.9), "test_interp_parse_line");
	assert_equal_double(i.get_bindings_dictionary()->get_value("b"), pow(3.9, 2), "test_interp_parse_line");
	assert_equal_double(i.get_bindings_dictionary()->get_value("c"), -0.23 * 3.9, "test_interp_parse_line");
	assert_equal_double(i.get_bindings_dictionary()->get_value("d"), 9.2, "test_interp_parse_line");
	assert_equal_double(i.get_bindings_dictionary()->get_value("e"), log(9.2), "test_interp_parse_line");
	assert_equal_double(i.get_bindings_dictionary()->get_value("f"), 9.2, "test_interp_parse_line");
	assert_equal_double(i.get_bindings_dictionary()->get_value("g"), DBL_MAX, "test_interp_parse_line");

	assert_true(i.get_var_types()->at("x") == VariableType::INPUT, "X should be an input", "test_interp_parse_line");
	assert_true(i.get_var_types()->at("w") == VariableType::WEIGHT, "W should be an weight", "test_interp_parse_line");
	assert_true(i.get_var_types()->at("y") == VariableType::EXP_OUTPUT, "Y should be an exp_output", "test_interp_parse_line");


	pass("test_interp_parse_line");

}

void test_interp_apply_binary_operation() {

	// simple errors
	assert_equal_double(apply_binary_operation(OperationType::INVALID_OPERATION, 2, 1), DBL_MIN, "test_interp_apply_binary_operation");
	assert_equal_double(apply_binary_operation(OperationType::ADD, DBL_MIN, 1), DBL_MIN, "test_interp_apply_binary_operation");
	assert_equal_double(apply_binary_operation(OperationType::MUL, DBL_MAX, DBL_MIN), DBL_MIN, "test_interp_apply_binary_operation");
	assert_equal_double(apply_binary_operation(OperationType::POW, DBL_MAX, 1), DBL_MAX, "test_interp_apply_binary_operation");
	assert_equal_double(apply_binary_operation(OperationType::LN, 3, 1), DBL_MIN, "test_interp_apply_binary_operation");

	// test correct cases
	assert_equal_double(apply_binary_operation(OperationType::ADD, 0.0, -123.1), -123.1, "test_interp_apply_binary_operation");
	assert_equal_double(apply_binary_operation(OperationType::SUB, 2.2, -1.9), 4.1, "test_interp_apply_binary_operation");
	assert_equal_double(apply_binary_operation(OperationType::MUL, 9.8, -0.21), -2.058, "test_interp_apply_binary_operation");
	assert_equal_double(apply_binary_operation(OperationType::POW, 2.2, -1.9), 0.22356, "test_interp_apply_binary_operation");

	// test errors with pow
	assert_equal_double(apply_binary_operation(OperationType::POW, -2, 9.2), DBL_MIN, "test_interp_apply_binary_operation");
	assert_equal_double(apply_binary_operation(OperationType::POW, 0, 0), 1, "test_interp_apply_binary_operation");
	assert_equal_double(apply_binary_operation(OperationType::POW, 0.0, -1.9), DBL_MIN, "test_interp_apply_binary_operation");

	pass("test_interp_apply_binary_operation");

}

void test_interp_apply_unary_operation() {

	// simple errors
	assert_equal_double(apply_unary_operation(OperationType::INVALID_OPERATION, 2), DBL_MIN, "test_interp_apply_unary_operation");
	assert_equal_double(apply_unary_operation(OperationType::ADD, DBL_MIN), DBL_MIN, "test_interp_apply_unary_operation");
	assert_equal_double(apply_unary_operation(OperationType::MUL, DBL_MAX), DBL_MAX, "test_interp_apply_unary_operation");
	assert_equal_double(apply_unary_operation(OperationType::ADD, 1), DBL_MIN, "test_interp_apply_unary_operation");
	
	// test correct cases
	assert_equal_double(apply_unary_operation(OperationType::LOGISTIC, -0.29), 1 / (1 + exp(0.29)), "test_interp_apply_unary_operation");
	assert_equal_double(apply_unary_operation(OperationType::LOGISTIC, 0), 0.5, "test_interp_apply_unary_operation");
	assert_equal_double(apply_unary_operation(OperationType::EXP, 2.31), exp(2.31), "test_interp_apply_unary_operation");
	assert_equal_double(apply_unary_operation(OperationType::EXP, 0), 1, "test_interp_apply_unary_operation");
	assert_equal_double(apply_unary_operation(OperationType::LN, 245.1), log(245.1), "test_interp_apply_unary_operation");
	assert_equal_double(apply_unary_operation(OperationType::LN, 1.0), 0, "test_interp_apply_unary_operation");

	// test errors with ln
	assert_equal_double(apply_unary_operation(OperationType::LN, -2), DBL_MIN, "test_interp_apply_unary_operation");
	assert_equal_double(apply_unary_operation(OperationType::LN, 0), DBL_MIN, "test_interp_apply_unary_operation");

	pass("test_interp_apply_unary_operation");

}


void run_interp_tests() {

	cout << "\nTesting Interpreter Class... " << endl << endl;

	test_interp_constructor_interpret_destructor();
	test_interp_parse_input_file();
	test_interp_parse_input_line();
	test_interp_accumulate_outputs();
	test_interp_parse_line();
	test_interp_apply_binary_operation();
	test_interp_apply_unary_operation();

	cout << "\nAll Interpreter Tests Passed." << endl << endl;
}
