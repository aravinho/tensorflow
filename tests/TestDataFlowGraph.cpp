#include <iostream>

#include "TestDataFlowGraph.h"
#include "TestUtilities.h"
#include "../src/DataFlowGraph.h"

using namespace std;


void test_dfg_constructor() {

	// test constructor initializes members correctly
	DataFlowGraph d;
	assert_equal_int(d.get_num_nodes(), 0, "test_dfg_constructor");
	assert_true(d.get_node("n") == NULL, "There are no nodes in this DFG yet", "test_dfg_constructor");
	assert_true(d.get_loss_node() == NULL, "There is no loss node in this DFG", "test_dfg_constructor");
	assert_equal_string(d.get_loss_var_name(), "", "test_dfg_constructor");

	pass("test_dfg_constructor");

}

void test_dfg_destructor() {

	// test destructor only deletes nodes in its map of nodes
	// also test that Node destructor does not delete its child nodes unless they are constants
	DataFlowGraph *d = new DataFlowGraph();
	d->add_node(new Node("a", false));
	d->add_node(new Node("b", false));
	d->add_node(new Node("c", false));

	Node *n = new Node("n", false);
	d->get_node("a")->set_child(n);
	n->add_parent(d->get_node("a"));

	delete d; delete n;

	pass("test_dfg_destructor");
}

void test_dfg_add_node() {

	DataFlowGraph *d = new DataFlowGraph();

	// can't add null node
	assert_true(d->add_node(NULL) == -1, "Cannot add a NULL node", "test_dfg_add_node");
	
	// basic add
	Node *n = new Node("n", false);
	n->set_type(VariableType::INTVAR);
	assert_true(d->add_node(n) == 0, "Add node should have succeeded", "test_dfg_add_node");
	assert_equal_int(d->get_num_nodes(), 1, "test_dfg_add_node");
	assert_true(d->get_node("n") == n, "Node N should've been retrieved", "test_dfg_add_node");

	// can't add same node twice
	Node *x = new Node("n", false);
	assert_true(d->add_node(x) == -1, "Can't add node with the same name", "test_dfg_add_node");

	// add loss node
	Node *loss = new Node("loss", false);
	loss->set_type(VariableType::LOSS);
	assert_equal_int(d->add_node(loss), 0, "test_dfg_add_node");
	assert_equal_string(d->get_loss_var_name(), "loss", "test_dfg_add_node");
	assert_true(d->get_loss_node() == loss, "Loss node should be LOSS", "test_dfg_add_node");
	assert_equal_int(loss->get_parents()->count(loss), 1, "test_dfg_add_node");
	assert_equal_int(loss->get_parent_names()->count("loss"), 1, "test_dfg_add_node");

	// add another loss node
	Node *loss_two = new Node("loss_two", false);
	loss_two->set_type(VariableType::LOSS);
	assert_true(d->add_node(loss_two) == -1, "Cannot have two loss nodes", "test_dfg_add_node");

	// cannot add constant node
	Node *c = new Node("0.23", true);
	assert_true(d->add_node(c) == -1, "Cannot add constant node to DFG", "test_dfg_add_node");

	delete d; delete c; delete loss_two; delete x;
	pass("test_dfg_add_node");

}

void test_dfg_get_node() {

	// test get_node returns null if not found
	DataFlowGraph d;
	Node *n = new Node("n", false);
	assert_true(d.get_node("n") == NULL, "No node with the name N", "test_dfg_get_node");

	// test basic add_node, get_node
	d.add_node(n);
	assert_true(d.get_node("n") == n, "Node N was retrieved", "test_dfg_get_node");

	// add another node with the same name
	Node *x = new Node("n", false);
	d.add_node(x);
	assert_true(d.get_node("n") == n, "Node N was retrieved", "test_dfg_get_node");

	delete x;
	pass("test_dfg_get_node");

}

void test_dfg_add_flow_edge() {

	DataFlowGraph *d = new DataFlowGraph();
	Node *parent = new Node("parent", false);
	Node *child = new Node("child", false);

	// parent not in DFG
	assert_false(d->add_flow_edge("child", "parent"), "Parent not found in DFG", "test_dfg_add_flow_edge");

	// parent in DFG but child not
	d->add_node(parent);
	assert_false(d->add_flow_edge("child", "parent"), "Child not found in DFG", "test_dfg_add_flow_edge");

	// check child's parent and parent's child got correctly set
	d->add_node(child);
	assert_true(d->add_flow_edge("child", "parent"), "Add flow edge should've succeeded", "test_dfg_add_flow_edge");
	assert_equal_int(child->get_parents()->count(parent), 1, "test_dfg_add_flow_edge");
	assert_equal_int(child->get_parent_names()->count("parent"), 1, "test_dfg_add_flow_edge");
	assert_true(parent->get_child_one() == child, "Parent's first child should be CHILD", "test_dfg_add_flow_edge");

	// add constant child
	assert_true(d->add_flow_edge("7", "child"), "Adding flow edge from constant to node should work", "test_dfg_add_flow_edge");
	assert_true(child->get_child_one_name() == "7", "CHILD's first child should be 7", "test_dfg_add_flow_edge");
	
	delete d;
	pass("test_dfg_add_flow_edge");

}

void test_dfg_get_num_nodes() {

	// test constant nodes are not added to DFG
	DataFlowGraph d;
	Node *a = new Node("a", false), *b = new Node("132", true), *c = new Node("c", false);
	d.add_node(a); d.add_node(b); d.add_node(c);
	d.add_flow_edge("c", "a");
	d.add_flow_edge("17", "a");
	assert_equal_int(d.get_num_nodes(), 2, "test_dfg_get_num_nodes");


	pass("test_dfg_get_num_nodes");

}

void test_dfg_get_loss_node() {

	// test get_loss_node when there is no loss node
	DataFlowGraph d;
	Node *a = new Node("a", false);
	a->set_type(VariableType::INTVAR);
	d.add_node(a);
	assert_true(d.get_loss_node() == NULL, "Loss node not seen", "test_dfg_get_loss_node");
	assert_equal_string(d.get_loss_var_name(), "", "test_dfg_get_loss_node");

	// test get_loss_node when there is a loss node
	Node *loss = new Node("loss", false);
	loss->set_type(VariableType::LOSS);
	d.add_node(loss);
	d.add_flow_edge("a", "d");
	assert_true(d.get_loss_node() == loss, "Loss node is LOSS", "test_dfg_get_loss_node");
	assert_equal_string(d.get_loss_var_name(), "loss", "test_dfg_get_loss_node");


	pass("test_dfg_get_loss_node");

}

void test_dfg_clear_all_markings() {

	// set arbitrary markings for 6 nodes in DFG
	// clear all, test they are all cleared
	DataFlowGraph *dfg = new DataFlowGraph();
	Node *a = new Node("a", false); dfg->add_node(a);
	Node *b = new Node("b", false); dfg->add_node(b);
	Node *c = new Node("c", false); dfg->add_node(c);
	Node *d = new Node("d", false); dfg->add_node(d);
	Node *e = new Node("e", false); dfg->add_node(e);
	Node *f = new Node("f", false); dfg->add_node(f);
	a->temporary_mark(); b->permanent_mark(); e->temporary_mark(); b->clear_mark();
	dfg->clear_all_markings();
	assert_true(a->is_unmarked(), "A must be unmarked", "test_dfg_clear_all_markings");
	assert_true(b->is_unmarked(), "B must be unmarked", "test_dfg_clear_all_markings");
	assert_true(c->is_unmarked(), "C must be unmarked", "test_dfg_clear_all_markings");
	assert_true(d->is_unmarked(), "D must be unmarked", "test_dfg_clear_all_markings");
	assert_true(e->is_unmarked(), "E must be unmarked", "test_dfg_clear_all_markings");
	assert_true(f->is_unmarked(), "F must be unmarked", "test_dfg_clear_all_markings");

	delete dfg;

	pass("test_dfg_clear_all_markings");

}

void test_dfg_top_sort() {
	/* Create a DFG that looks like this:

	C
	 \
	  B   7
	   \ /
	    A

	*/

	DataFlowGraph d;
	Node *a = new Node("a", false);
	a->set_type(VariableType::LOSS);
	Node *b = new Node("b", false);
	d.add_node(a);
	d.add_node(b);
	d.add_flow_edge("b", "a");
	d.add_flow_edge("7", "a");
	Node *c = new Node("c", false);
	d.add_node(c);
	d.add_flow_edge("c", "b");

	list<Node *> *sorted_nodes = new list<Node *>();
	d.top_sort(sorted_nodes);

	// test all nodes are permanent marked after sort
	assert_true(a->is_permanent_marked(), "A must be permanent marked", "test_dfg_top_sort");
	assert_true(b->is_permanent_marked(), "A must be permanent marked", "test_dfg_top_sort");
	assert_true(c->is_permanent_marked(), "A must be permanent marked", "test_dfg_top_sort");

	// test the order is A, B, C
	int i = 0;
	for (list<Node *>::iterator it = sorted_nodes->begin(); it != sorted_nodes->end(); ++it) {
		if (i == 0) assert_equal_string((*it)->get_name(), "a", "test_dfg_top_sort");
		if (i == 1) assert_equal_string((*it)->get_name(), "b", "test_dfg_top_sort");
		if (i == 2) assert_equal_string((*it)->get_name(), "c", "test_dfg_top_sort");
		i++;
	}

	pass("test_dfg_top_sort");

}


void run_dfg_tests() {
	cout << "\nTesting DataFlowGraph Class... " << endl << endl;

	test_dfg_constructor();
	test_dfg_destructor();
	test_dfg_add_node();
	test_dfg_get_node();
	test_dfg_add_flow_edge();
	test_dfg_get_num_nodes();
	test_dfg_get_loss_node();
	test_dfg_clear_all_markings();
	test_dfg_top_sort();

	cout << "\nAll DataFlowGraph Tests Passed." << endl << endl;
}