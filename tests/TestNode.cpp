#include <iostream>

#include "TestNode.h"
#include "TestUtilities.h"
#include "../Node.h"

void test_node_constructor() {

	// create basic node
	Node n;
	assert_equal_int(n.get_num_children(), 0, "test_node_constructor");
	assert_equal_int(n.get_mark(), 0, "test_node_constructor");
	assert_true(n.get_type() == VariableType::INVALID_VAR_TYPE, "Initial var type should be invalid", "test_node_constructor");
	assert_true(n.get_operation() == OperationType::INVALID_OPERATION, "Initial operation type should be invalid", "test_node_constructor");
	assert_equal_int(n.get_parents()->size(), 0, "test_node_constructor");
	assert_true(n.get_child_one() == NULL, "Child one should be NULL initially", "test_node_constructor");
	assert_true(n.get_child_two() == NULL, "Child two should be NULL initially", "test_node_constructor");
	assert_equal_string(n.get_child_one_name(), "", "test_node_constructor");
	assert_equal_string(n.get_child_two_name(), "", "test_node_constructor");
	assert_false(n.is_constant(), "Node should not be constant", "test_node_constructor");

	// create constant node
	Node const_node("-7.3", true);
	assert_true(const_node.is_constant(), "node should be constant", "test_node_constructor");
	assert_equal_string(const_node.get_name(), "-7.3", "test_node_constructor");
	assert_true(const_node.get_type() == VariableType::CONSTANT, "node variable type should be constant", "test_node_constructor");
	assert_equal_int(const_node.get_num_children(), 0, "test_node_constructor");
	assert_equal_int(const_node.get_mark(), 0, "test_node_constructor");
	assert_true(n.get_operation() == OperationType::INVALID_OPERATION, "Initial operation type should be invalid", "test_node_constructor");
	assert_equal_int(n.get_parents()->size(), 0, "test_node_constructor");
	assert_true(n.get_child_one() == NULL, "Child one should be NULL initially", "test_node_constructor");
	assert_true(n.get_child_two() == NULL, "Child two should be NULL initially", "test_node_constructor");
	assert_equal_string(n.get_child_one_name(), "", "test_node_constructor");
	assert_equal_string(n.get_child_two_name(), "", "test_node_constructor");

	pass("test_node_constructor");

}

void test_node_destructor() {

	// test that Node destructor only frees constant children
	Node *n = new Node("n", false);
	Node *fc = new Node("fc", false);
	Node *sc = new Node("17", true);
	n->set_child(fc);
	n->set_child(sc);
	fc->add_parent(n);
	sc->add_parent(n);
	delete n;
	delete fc;
	pass("test_node_destructor");
}

void test_node_name() {

	// test set name sets name
	Node *n = new Node();
	n->set_name("lala");
	assert_equal_string(n->get_name(), "lala", "test_node_name");

	// test constructor sets name, set_name changes it
	Node x("x", false);
	assert_equal_string(x.get_name(), "x", "test_node_name");
	x.set_name("y");
	assert_equal_string(x.get_name(), "y", "test_node_name");

	delete n;
	pass("test_node_name");
}

void test_node_type() {

	// test type is initially invalid
	Node a("a", false);
	assert_true(a.get_type() == VariableType::INVALID_VAR_TYPE, "A's initial var type is invalid", "test_node_type");

	// test get_type reflects set_type
	a.set_type(VariableType::INTVAR);
	assert_true(a.get_type() == VariableType::INTVAR, "A's var Type should be INTVAR", "test_node_type");
	a.set_type(VariableType::LOSS);
	assert_true(a.get_type() == VariableType::LOSS, "A's var type should be LOSS", "test_node_type");

	// test constant nodes can't have their types changed
	Node b("9", true);
	assert_true(b.get_type() == VariableType::CONSTANT, "B's initial var type should be CONSTANT", "test_node_type");
	b.set_type(VariableType::LOSS);
	assert_true(b.get_type() == VariableType::CONSTANT, "B's initial var type should be CONSTANT", "test_node_type");

	pass("test_node_type");
}

void test_node_is_constant() {

	// test constant constructor sets is_constant
	Node a("0.9", true);
	assert_true(a.is_constant(), "Node A should be constant", "test_node_is_constant");
	Node *b = new Node("b", false);
	assert_false(b->is_constant(), "Node B should not be constant", "test_node_is_constant");

	delete b;
	pass("test_node_is_constant");
}

void test_node_operation() {

	// test initial operation is invalid
	Node a("a", false);
	assert_true(a.get_operation() == OperationType::INVALID_OPERATION, "A's initial operation type should be Invalid", "test_node_operation");
	
	// test get_operation reflects set_operation
	a.set_operation(OperationType::ADD);
	assert_true(a.get_operation() == OperationType::ADD, "A's operation type should be ADD", "test_node_operation");
	a.set_operation(OperationType::LOGISTIC);
	assert_true(a.get_operation() == OperationType::LOGISTIC, "A's operation type should be LOGISTIC", "test_node_operation");

	// test constant nodes cannot have their operation changed
	Node b("9.0", true);
	b.set_operation(OperationType::MUL);
	assert_true(b.get_operation() == OperationType::INVALID_OPERATION, "Constant nodes can't have their operation set", "test_node_operation");

	pass("test_node_operation");
}

void test_node_parent() {

	// test node has no parents initially
	Node a("a", false);
	assert_equal_int(a.get_parents()->size(), 0, "test_node_parent");

	// test adding null parent doesn't work
	assert_false(a.add_parent(NULL), "Shouldn't be able to set a NULL parent", "test_node_parent");
	
	// test adding constant parent doesn't work
	Node *x = new Node("0", true);
	assert_false(a.add_parent(x), "shouldn't be able to set a constant parent", "test_node_parent");
	
	// test adding input/weight/exp_output parent doesn't work
	Node *y = new Node("y", false);
	y->set_type(VariableType::INPUT);
	assert_false(a.add_parent(y), "shouldn't be able to set an input/weight/exp_output parent", "test_node_parent");
	y->set_type(VariableType::WEIGHT);
	assert_false(a.add_parent(y), "shouldn't be able to set an input/weight/exp_output parent", "test_node_parent");
	y->set_type(VariableType::EXP_OUTPUT);
	assert_false(a.add_parent(y), "shouldn't be able to set an input/weight/exp_output parent", "test_node_parent");

	// test basic add parent works
	Node *parent1 = new Node("parent1", false), *parent2 = new Node("parent2", false);
	assert_true(a.add_parent(parent1), "Should be able to set parent1", "test_node_parent");
	assert_equal_int(a.get_parents()->size(), 1, "test_node_parent");
	assert_equal_int(a.get_parent_names()->size(), 1, "test_node_parent");
	assert_true(a.add_parent(parent2), "Should be able to set parent2", "test_node_parent");
	assert_equal_int(a.get_parent_names()->size(), 2, "test_node_parent");

	set<Node *> *parents = a.get_parents();
	set<string> *parent_names = a.get_parent_names();
	set<string> expected_parents = {"parent1", "parent2"};
	for (set<Node *>::iterator it = parents->begin(); it != parents->end(); ++it) {
		Node *p = *it;
		assert_true(expected_parents.count(p->get_name()) == 1, "parent should be found", "test_node_parent");
	}
	for (set<string>::iterator it = parent_names->begin(); it != parent_names->end(); ++it) {
		assert_true(expected_parents.count(*it) == 1, "parent name should be found", "test_node_parent");
	}


	// test loss nodes cannot have non-loss parents
	Node loss("loss", false);
	loss.set_type(VariableType::LOSS);
	Node *z = new Node("z", false);
	assert_false(loss.add_parent(z), "Loss nodes cannot have parents", "test_node_parent");

	// test loss node can have themselves set as their parents
	assert_true(loss.add_parent(&loss), "Loss nodes' parents must be themselves", "test_node_parent");

	delete x; delete y; delete z; delete parent1; delete parent2;
	pass("test_node_parent");

}

void test_node_children() {

	Node a("a", false);
	Node *child_one = new Node("child_one", false);
	Node *child_two = new Node("23.0", true);

	// test cannot set a null child
	assert_false(a.set_child(NULL), "Cannot set a null child", "test_node_children");

	// test that an input/weight/exp_output node cannot have a child
	a.set_type(VariableType::INPUT);
	assert_false(a.set_child(child_one), "Input/Weight/exp_output nodes cannot have children", "test_node_children");
	a.set_type(VariableType::WEIGHT);
	assert_false(a.set_child(child_one), "Input/Weight/exp_output nodes cannot have children", "test_node_children");
	a.set_type(VariableType::EXP_OUTPUT);
	assert_false(a.set_child(child_one), "Input/Weight/exp_output nodes cannot have children", "test_node_children");

	a.set_type(VariableType::INTVAR);

	// test cannot set a loss node as a child
	Node *loss = new Node("loss", false);
	loss->set_type(VariableType::LOSS);
	assert_false(a.set_child(loss), "Cannot set a loss node as a child", "test_node_children");

	// test basic add child
	assert_true(a.set_child(child_one), "Child one should be set successfully", "test_node_children");
	assert_equal_int(a.get_num_children(), 1, "test_node_children");
	assert_equal_string(a.get_child_two_name(), "", "test_node_children");

	// test child one and child two added successfully (child_two constant)
	assert_true(a.set_child(child_two), "Child two should be set successfully", "test_node_children");
	assert_equal_int(a.get_num_children(), 2, "test_node_children");
	assert_equal_string(a.get_child_one_name(), "child_one", "test_node_children");
	assert_equal_string(a.get_child_two_name(), "23.0", "test_node_children");
	assert_true(a.get_child_one() == child_one, "Child one should be child_one node", "test_node_children");
	assert_true(a.get_child_two() == child_two, "Child two should be child_two node", "test_node_children");
	assert_true(a.has_child_with_name("child_one"), "A has a child with name child_one", "test_node_children");
	assert_true(a.has_child_with_name("23.0"), "A has a child with name child_two", "test_node_children");

	// test cannot have more than 2 children
	Node *child_three = new Node("child_three", false);
	assert_false(a.set_child(child_three), "Cannot have three children", "test_node_children");

	Node *x = new Node("x", false), *y = new Node("-89", true);

	// test constant nodes cannot have children
	assert_false(child_two->set_child(x), "Constant nodes cannot have children", "test_node_children");
	assert_true(child_one->set_child(y), "Child one should be able to have a child", "test_node_children");

	delete child_one; delete loss; delete child_three; delete x;		// do not delete constant nodes
	pass("test_node_children");
}

void test_node_mark() {

	Node a("a", false);

	// test mark is initially 0.
	assert_equal_int(a.get_mark(), 0, "test_node_mark");
	assert_true(a.is_unmarked(), "A should be unmarked", "test_node_mark");
	assert_false(a.is_temporary_marked(), "A should not be temporary marked", "test_node_mark");
	assert_false(a.is_permanent_marked(), "A should not be permanent marked", "test_node_mark");

	// test temporary mark works
	a.temporary_mark();
	assert_equal_int(a.get_mark(), 1, "test_node_mark");
	assert_false(a.is_unmarked(), "A should not be unmarked", "test_node_mark");
	assert_true(a.is_temporary_marked(), "A should be temporary marked", "test_node_mark");
	assert_false(a.is_permanent_marked(), "A should not be permanent marked", "test_node_mark");

	// test permanent mark works
	a.permanent_mark();
	assert_equal_int(a.get_mark(), 2, "test_node_mark");
	assert_false(a.is_unmarked(), "A should not be unmarked", "test_node_mark");
	assert_false(a.is_temporary_marked(), "A should not be temporary marked", "test_node_mark");
	assert_true(a.is_permanent_marked(), "A should be permanent marked", "test_node_mark");

	// test clear mark works
	a.clear_mark();
	assert_equal_int(a.get_mark(), 0, "test_node_mark");
	assert_true(a.is_unmarked(), "A should be unmarked", "test_node_mark");
	assert_false(a.is_temporary_marked(), "A should not be temporary marked", "test_node_mark");
	assert_false(a.is_permanent_marked(), "A should not be permanent marked", "test_node_mark");	

	pass("test_node_mark");

}


void run_node_tests() {
	cout << "\nTesting Node Class... " << endl << endl;
	test_node_constructor();
	test_node_destructor();
	test_node_name();
	test_node_type();
	test_node_is_constant();
	test_node_operation();
	test_node_parent();
	test_node_children();
	test_node_mark();
	cout << "\nAll Node Tests Passed." << endl << endl;
}