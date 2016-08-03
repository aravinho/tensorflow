#include <string>
#include <cfloat>

#include "Node.h"

using namespace std;


/* ----------- Constructors/Destructors ------------- */

Node::Node() {
	name = "";
	constant_value = FLT_MIN;
	type = VariableType::INVALID_VAR_TYPE;
	operation = OperationType::INVALID_OPERATION;

    parents = new set<Node *>();
    parent_names = new set<string>();

    child_one_name = "";
    child_two_name = "";
    child_one = NULL;
    child_two = NULL;

    num_children = 0;
	mark = 0;
}


Node::Node(string node_name, bool is_constant) : Node() {
	name = node_name;
	if (is_constant) {
		constant_value = stof(node_name);
		set_type(VariableType::CONSTANT);
	}	
}


Node::~Node() {
	if (child_one && child_one->is_constant()) delete child_one;
	if (child_two && child_two->is_constant()) delete child_two;
}


/* ----------- Basic Getters/Setters ---------------- */

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
	if (this->is_constant()) return;
	type = new_type;
}

bool Node::is_constant() const {
	return type == VariableType::CONSTANT;
}

OperationType Node::get_operation() const {
	return operation;
}
void Node::set_operation(OperationType new_operation) {
	if (this->is_constant()) return;
	operation = new_operation;
}


/* ----------- Parent Methods --------------- */


set<Node *> *Node::get_parents() const {
	return parents;
}
set<string> *Node::get_parent_names() const {
	return parent_names;
}
bool Node::has_parent() const {
	return parents->size() > 0;
}
bool Node::add_parent(Node *new_parent) {
	if (new_parent == NULL || new_parent->is_constant()) return false;
	if (new_parent->get_type() == VariableType::INPUT
		|| new_parent->get_type() == VariableType::WEIGHT 
		|| new_parent->get_type() == VariableType::EXP_OUTPUT) return false;
	if (this->get_type() == VariableType::LOSS
		&& new_parent != this) return false;

	parents->insert(new_parent);
	parent_names->insert(new_parent->get_name());
	return true;
}


/* ------------ Child Methods ---------------- */

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
	if (new_child == NULL || num_children == 2 || this->is_constant()) return false;
	if (this->get_type() == VariableType::INPUT
		|| this->get_type() == VariableType::WEIGHT 
		|| this->get_type() == VariableType::EXP_OUTPUT) return false; 
	if (new_child->get_type() == VariableType::LOSS) return false;
	
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


/* ---------------- Topological Sort Mark Methods -------------- */

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

