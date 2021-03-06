#include <string>

#include "DataFlowGraph.h"


using namespace std;


/* --------------- Constructor/Destructor ---------------- */

DataFlowGraph::DataFlowGraph() {
	nodes = new unordered_map<string, Node *>();
	num_nodes = 0;
	loss_node_added = false;
	loss_node = NULL;
	loss_var_name = "";
}


DataFlowGraph::~DataFlowGraph() {
	for (unordered_map<string, Node*>::iterator it = nodes->begin();
		it != nodes->end(); ++it) {
		delete it->second;
	}

	delete nodes;
}


/* ---------------- Nodes and Edges ----------- */

int DataFlowGraph::add_node(Node *node) {
	if (node == NULL) {
		return -1;
	}

	if (node->is_constant()) return -1;

	// If we are adding the loss node, make sure to set the loss node's parent to be itself.
	if (node->get_type() == VariableType::LOSS) {
		if (loss_node_added) {
			return -1;
		}

		loss_node = node;
		loss_var_name = node->get_name();
		loss_node_added = true;
		node->add_parent(node);
	}

	if (nodes->count(node->get_name()) > 0) {
		return -1;
	}

	nodes->insert(make_pair(node->get_name(), node));
	num_nodes++;
	return 0;
}


Node *DataFlowGraph::get_node(const string& name) const {
	if (nodes->count(name) == 0) {
		return NULL;
	}
	return nodes->at(name);
}


bool DataFlowGraph::add_flow_edge(const string& child_name, const string& parent_name) {
	Node *parent = get_node(parent_name);
	if (parent == NULL) {
		return false;
	}

	Node *child; 
	if (is_constant(child_name)) {
		child = new Node(string(child_name), true);
	} else {
		child = get_node(child_name);
	}
	
	if (child == NULL) {
		return false;
	}

	bool success = child->add_parent(parent);
	success = success && parent->set_child(child);
	return success;	
}


int DataFlowGraph::get_num_nodes() const {
	return num_nodes;
}



/* ---------------- Loss Node ------------------- */

string DataFlowGraph::get_loss_var_name() const {
	return loss_var_name;
}

Node *DataFlowGraph::get_loss_node() const {
	return loss_node;
}



/* ---------------- Topological Sort ------------- */

void DataFlowGraph::clear_all_markings() {
	for (unordered_map<string, Node *>::iterator it = nodes->begin(); it != nodes->end(); ++it) {
		it->second->clear_mark();
	}
}

void DataFlowGraph::top_sort(list<Node *> *sorted_nodes) {
	clear_all_markings();
	top_sort_visit(loss_node, sorted_nodes);
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

