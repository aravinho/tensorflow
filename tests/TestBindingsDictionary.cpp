#include <iostream>
#include <cfloat>

#include "TestBindingsDictionary.h"
#include "../BindingsDictionary.h"
#include "TestUtilities.h"

using namespace std;


void test_bd_constructor() {

	// test constructor initializes bindings map
	BindingsDictionary b;
	assert_equal_int(b.bindings->size(), 0, "test_bd_constructor");

	pass("test_bd_constructor");
}

void test_bd_destructor() {

	// test destructor deletes the bindings map without problems
	BindingsDictionary *b = new BindingsDictionary();
	delete b;

	pass("test_bd_destructor");
}

void test_bd_add_variable() {

	// test basic add works
	BindingsDictionary b;
	assert_equal_int(b.add_variable("x"), 0, "test_bd_add_variable");

	// test dummy value is FLT_MAX
	assert_true(b.get_value("x") == FLT_MAX, "Dummy value should be FLT_MAX", "test_bd_add_variable");

	// test cannot double add
	assert_equal_int(b.add_variable("x"), -1, "test_bd_add_variable");

	pass("test_bd_add_variable");

}

void test_bd_bind_value() {

	BindingsDictionary *b = new BindingsDictionary();

	// test bind fails if not declared
	assert_equal_int(b->bind_value("x", 12), -1, "test_bd_bind_value");
	assert_true(b->get_value("x") == FLT_MIN, "FLT_MIN if not yet added", "test_bd_bind_value");

	assert_equal_int(b->add_variable("x"), 0, "test_bd_bind_value");

	// test bind fails if value is FLT_MIN or FLT_MAX
	assert_equal_int(b->bind_value("x", FLT_MIN), -1, "test_bd_bind_value");
	assert_equal_int(b->bind_value("x", FLT_MAX), -1, "test_bd_bind_value");

	// test bind works correctly
	assert_equal_int(b->bind_value("x", 12), 0, "test_bd_bind_value");

	// test bind fails if already defined
	assert_equal_int(b->bind_value("x", 17), -1, "test_bd_bind_value");

	delete b;
	pass("test_bd_bind_value");

}

void test_bd_get_value() {

	BindingsDictionary b;

	// test returns FLT_MIN if not declared
	assert_true(b.get_value("x") == FLT_MIN, "If not declared, should return FLT_MIN", "test_bd_get_value");

	// test returns FLT_MAX if declared but not defined
	b.add_variable("x");
	assert_true(b.get_value("x") == FLT_MAX, "If declared but not defined, should return FLT_MAX", "test_bd_get_value");

	// test returned correct value if defined
	b.bind_value("x", -2342.2);
	assert_equal_float(b.get_value("x"), -2342.2, "test_bd_get_value");

	pass("test_bd_get_value");

}

void test_bd_has_been_declared() {

	BindingsDictionary b;

	// test returns false if not declared
	assert_false(b.has_been_declared("x"), "Should return false if not declared", "test_bd_has_been_declared");
	
	// test returns true if declared
	b.add_variable("x");
	assert_true(b.has_been_declared("x"), "Should return true if declared", "test_bd_has_been_declared");

	// test returns true if declared and defined
	b.bind_value("x", 0.0);
	assert_true(b.has_been_declared("x"), "Should return true if defined", "test_bd_has_been_declared");

	pass("test_bd_has_been_declared");
}

void test_bd_has_been_defined() {

	BindingsDictionary b;

	// test returns false if not declared
	assert_false(b.has_been_defined("x"), "Should return false if not declared", "test_bd_has_been_declared");
	
	// test returns false if declared
	b.add_variable("x");
	assert_false(b.has_been_defined("x"), "Should return false if declared", "test_bd_has_been_declared");
	
	// test returns true only if declared and defined
	b.bind_value("x", -23);
	assert_true(b.has_been_defined("x"), "Should return true if declared", "test_bd_has_been_declared");

	pass("test_bd_has_been_defined");
}


void run_bd_tests() {

	cout << "\nTesting BindingsDictionary Class... " << endl << endl;

	test_bd_constructor();
	test_bd_destructor();
	test_bd_add_variable();
	test_bd_bind_value();
	test_bd_get_value();
	test_bd_has_been_declared();
	test_bd_has_been_defined();

	cout << "\nAll BindingsDictionary Tests Passed." << endl << endl;

}
