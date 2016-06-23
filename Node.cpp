#include <string>
#include <iostream>
#include <fstream>
#include "Node.h"
#include "utilities.h"


using namespace std;

/* A node in the dependency graph. */


/* ----------- Constructors ------------- */

Node::Node() {
	num_children = 0;
	mark = 0;
}

/* Creates a node with the given NAME.
 * If IS_CONSTANT is set, this node will represent a constant (a float).
 */
Node::Node(string node_name, bool is_constant) {
	name = node_name;

	if (is_constant) {
		constant_value = stof(node_name);
		set_type(VariableType::CONSTANT);
	}

	num_children = 0;
	mark = 0;
}


/* ----------- Basic Info ---------------- */

string Node::get_name() const {
	return name;
}
void Node::set_name(string new_name) {
	name = new_name;
}
  
VariableType Node::get_type() const {
	return type;
}
void Node::set_type(VariableType new_type) {
	type = new_type;
}

bool Node::is_constant() const {
	return type == VariableType::CONSTANT;
}

OperationType Node::get_operation() const {
	return operation;
}
void Node::set_operation(OperationType new_operation) {
	operation = new_operation;
}


/* ----------- Parent Info --------------- */

string Node::get_parent_name() const {
	return parent_name;
}
Node *Node::get_parent() const {
	return parent;
}
bool Node::set_parent(Node *new_parent) {
	if (new_parent == NULL) {
		return false;
	}
	parent = new_parent;
	parent_name = new_parent->get_name();
	return true;
}


/* ------------ Child Info ---------------- */

string Node::get_child_one_name() const {
	return child_one_name;
}
string Node::get_child_two_name() const {
	return child_two_name;
}
Node *Node::get_child_one() const {
	return child_one;
}
Node *Node::get_child_two() const {
	return child_two;
}

bool Node::set_child(Node *new_child) {
	if (new_child == NULL || num_children == 2) {
		return false;
	}
	
	if (num_children == 0) {
		child_one = new_child;
		child_one_name = new_child->get_name();
	} else if (num_children == 1) {
		child_two = new_child;
		child_two_name = new_child->get_name();
	}
	num_children++;
	return true;
}

bool Node::has_child_with_name(const string& child_name) const {
	return (child_one_name.compare(child_name) == 0 || child_two_name.compare(child_name) == 0);
}
int Node::get_num_children() const {
	return num_children;
}


/* ---------------- Topological Sort Mark Info -------------- */

int Node::get_mark() const {
	return mark;
}
void Node::clear_mark() {
	mark = 0;
}
void Node::temporary_mark() {
	mark = 1;
}
void Node::permanent_mark() {
	mark = 2;
}
bool Node::is_unmarked() const {
	return mark == 0;
}
bool Node::is_temporary_marked() const {
	return mark == 1;
}
bool Node::is_permanent_marked() const {
	return mark == 2;
}

