#include <string>
#include <iostream>
#include <fstream>
#include "Node.h"

using namespace std;

/* A node in the dependency graph. */

Node::Node() {

}

/* Creates a node with the given name.
 */
Node::Node(char *node_name, bool is_constant) {
	strcpy(name, node_name);
	mark = 0;
	if (is_constant) {
		constant_value = stof(node_name);
		strcpy(type, "constant");
	}
}

/* Creates a "constant node" with the given value.
 * Names this node "constant", and sets its type to Constant.
 * There can be multiple constant nodes in a Data Flow Graph.
 */
/*Node::Node(char *node_name, bool is_constant) {
	constant_value = constant;
	strcpy(name, "constant");
	strcpy(type, "constant");
	mark = 0;
}*/



char *Node::get_name() {
	return name;
}
void Node::set_name(char *new_name) {
	strcpy(name, new_name);
}
  
//enum Node::variable_type_t get_type();
char *Node::get_type() {
	return type;
}
void Node::set_type(char *new_type) {
	strcpy(type, new_type);
}
bool Node::is_constant() {
	return (strcmp(type, "constant") == 0);
}



char *Node::get_parent_name() {
	return parent_name;
}
Node *Node::get_parent() {
	return parent;
}
bool Node::set_parent(Node *new_parent) {
	if (new_parent == NULL) {
		return false;
	}
	parent = new_parent;
	strcpy(parent_name, new_parent->get_name());
	return true;
}


    
//enum Node::operation_type_t get_operation();
char *Node::get_operation() {
	return operation;
}
void Node::set_operation(char *new_operation) {
	strcpy(operation, new_operation);
}




char *Node::get_child_one_name() {
	return child_one_name;
}
char *Node::get_child_two_name() {
	return child_two_name;
}
Node *Node::get_child_one() {
	return child_one;
}
Node *Node::get_child_two() {
	return child_two;
}

/* Sets a child of the current node to be the given node NEW_CHILD.
 * If the current node has no children, NEW_CHILD becomes Child 1.
 * If the current node has no children, NEW_CHILD becomes Child 2.
 * If the current node has two children already, this method returns false and nothing happens.
 * This method returns true otherwise.
 */
bool Node::set_child(Node *new_child) {
	if (num_children == 2) {
		return false;
	}

	if (num_children == 0) {
		child_one = new_child;
		strcpy(child_one_name, new_child->get_name());
	} else if (num_children == 1) {
		child_two = new_child;
		strcpy(child_two_name, new_child->get_name());
	}
	num_children++;
	return true;
}

bool Node::has_child_with_name(char *child_name) {
	return (strcmp(child_one_name, child_name) == 0) || (strcmp(child_two_name, child_name) == 0);
}


int Node::get_num_children() {
	return num_children;
}

int Node::get_mark() {
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
bool Node::is_unmarked() {
	return mark == 0;
}
bool Node::is_temporary_marked() {
	return mark == 1;
}
bool Node::is_permanent_marked() {
	return mark == 2;
}

