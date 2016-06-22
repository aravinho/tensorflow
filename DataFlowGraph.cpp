#include <iostream>
#include <fstream>
#include <string>
#include<list>

#include "Compiler.h"
#include "DataFlowGraph.h"
#include "Node.h"

using namespace std;

/* Returns whether the given name can be parsed as a constant (a float).
 * The try-catch block is because the stof method throws an exception if the given string cannot be parsed as a float.
 */
bool is_constant(char *name) {
	size_t size;
	float f;

	try {
  		f = stof(string(name), &size);
	} catch(const exception& e) {
		return false;
	}

	if (size == strlen(name)) {
		return true;
	}

	return false;
	
}

/* Default constructor.
 * Initializes the map of nodes, and the number of nodes.
 */
DataFlowGraph::DataFlowGraph() {
	nodes = new unordered_map<string, Node *>();
	num_nodes = 0;
}

/* Adds a node to the Data Flow Graph.
 * Inserts into the map of nodes a mapping from node-name to node.
 * If the given node is the loss node, makes a note of this.
 * Returns true if the node was successfully added.
 * Returns false if there is already a node by this name in the graph.
 * This method will not add the given node if there is an existing node by the same name.
 */
bool DataFlowGraph::add_node(Node *node) {
	if (node == NULL) {
		return false;
	}

	if (strcmp(node->get_type(), "loss") == 0) {
		loss_node = node;
		strcpy(loss_var_name, node->get_name());
		node->set_parent(node);
	}

	if (nodes->count(string(node->get_name())) > 0) {
		return false;
	}

	nodes->insert(make_pair(string(node->get_name()), node));
	num_nodes++;
	return true;
}

/* Returns a Node pointer for the node corresponding to the given NAME.
 * Returns NULL if a bad name (NULL or empty) is given, or if there is no node by the name in the Data Flow Graph.
 */
Node *DataFlowGraph::get_node(char *name) {
	if (name == NULL || strcmp(name, "") == 0) {
		return NULL;
	}

	if (nodes->count(string(name)) == 0) {
		return NULL;
	}
	return nodes->at(string(name));
}

/* Binds a child node to its parent.
 * A child node feeds its output to the parent.
 * The data "flows" from child to parent.
 * However, the edge can thought to be directed from parent to child.
 * This is because when evaluating partial derivatives, we must visit the parent before the child.
 * Returns false if there is no node found for the given CHILD_NAME or PARENT_NAME.
 * Returns true otherwise.
 */
bool DataFlowGraph::add_flow_edge(char *child_name, char *parent_name) {
	Node *parent = get_node(parent_name);
	if (parent == NULL) {
		return false;
	}

	Node *child;
	if (is_constant(child_name)) {
		child = new Node(child_name, true);
	} else {
		child = get_node(child_name);
	}
	
	if (child == NULL) {
		return false;
	}

	bool success = child->set_parent(parent);
	success = success && parent->set_child(child);
	return success;	
}

/* Returns the number of nodes in this Data Flow Graph.
 * Constant nodes are not included in this, so this number represents the number of "variables".
 */
int DataFlowGraph::get_num_nodes() {
	return num_nodes;
}

char *DataFlowGraph::get_loss_var_name() {
	return loss_var_name;
}

Node *DataFlowGraph::get_loss_node() {
	return loss_node;
}

/* Returns the map of nodes for this Data Flow Graph.
 */
unordered_map<string, Node*>* DataFlowGraph::get_all_nodes() {
	return nodes;
}

/* Sets all nodes in the Data Flow Graph to unmarked.
 * Used in preparation for topological sort.
 */
void DataFlowGraph::clear_all_markings() {
	for (unordered_map<string, Node *>::iterator it = nodes->begin(); it != nodes->end(); ++it) {
		it->second->clear_mark();
	}
}


void DataFlowGraph::top_sort_visit(Node *node, list<Node *> *sorted_nodes) {
	if (node == NULL || node->is_constant() || node->is_temporary_marked() || node->is_permanent_marked()) {
		return;
	}

	if (node->is_unmarked()) {
		node->temporary_mark();
		top_sort_visit(node->get_child_one(), sorted_nodes);
		top_sort_visit(node->get_child_two(), sorted_nodes);
		node->permanent_mark();
		sorted_nodes->push_front(node);
	}
}

void DataFlowGraph::top_sort(list<Node *> *sorted_nodes) {
	clear_all_markings();

	for (unordered_map<string, Node *>::iterator it = nodes->begin(); it != nodes->end(); ++it) {
		if (it->second->is_unmarked()) {
			top_sort_visit(it->second, sorted_nodes);
		}
	}
}