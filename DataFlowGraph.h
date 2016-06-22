#ifndef DATAFLOWGRAPH_H
#define DATAFLOWGRAPH_H

#include <unordered_map>

#include "Compiler.h"
#include "Node.h"

using namespace std;

class DataFlowGraph {

	unordered_map<string, Node*>* nodes;
	Node *loss_node;
	char loss_var_name[50];
	int num_nodes;

public:
	DataFlowGraph();
	bool add_node(Node *node);
	Node *get_node(char *name);
	bool add_flow_edge(char *child_name, char *parent_name);
	int get_num_nodes();

	char *get_loss_var_name();
	Node *get_loss_node();
	unordered_map<string, Node*>* get_all_nodes();

	void top_sort(list<Node *> *sorted_nodes);
	void clear_all_markings();
	void top_sort_visit(Node *node, list<Node *> *sorted_nodes);
};



#endif
