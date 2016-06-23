#ifndef DATAFLOWGRAPH_H
#define DATAFLOWGRAPH_H

#include <unordered_map>

#include "Compiler.h"
#include "Node.h"
#include "utilities.h"


using namespace std;

class DataFlowGraph {

	unordered_map<string, Node*>* nodes;
	Node *loss_node;
	string loss_var_name;
	int num_nodes;

public:

	/* Returns whether the given name can be parsed as a constant (a float).
 	 * The try-catch block is because the stof method throws an exception if the given string cannot be parsed as a float.
 	 */
	DataFlowGraph();



	/* Adds a node to the Data Flow Graph.
 	 * Stores a mapping from node-name to node.
 	 * If the given node is the loss node, makes a note of this.
 	 * Returns true if the node was successfully added.
 	 * Returns false if there is already a node by this name in the graph.
 	 * This method will not add the given node if there is an existing node by the same name.
 	 */
	bool add_node(Node *node);

	/* Returns a pointer to the node corresponding to the given NAME.
 	 * Returns NULL if there is no node by the name in the Data Flow Graph.
 	 */
	Node *get_node(const string& name) const;

	/* Binds a child node to its parent, by creating an edge from parent to child.
 	 * A child node feeds its output to the parent, so data "flows" from child to parent.
 	 * However, we direct the edge from parent to child.
 	 * This is because when evaluating partial derivatives, we must visit the parent before the child.
 	 * Thus, the "dependency graph edge" must go from parent to child.
 	 * Returns false if there is no node found for the given CHILD_NAME or PARENT_NAME.
 	 * Returns false if both of the parent's children have already been set.
 	 * Returns true otherwise.
 	 */
	bool add_flow_edge(const string& child_name, const string& parent_name);
	


	/* Returns the map of nodes for this Data Flow Graph.
 	 */
	const unordered_map<string, Node*>* get_all_nodes() const;

	/* Returns the number of nodes in this Data Flow Graph.
 	 * Constant nodes are not included in this, so this number represents the number of variables in the computation.
 	 */
	int get_num_nodes() const;



	string get_loss_var_name() const;
	Node *get_loss_node() const;

	

	/* Sets all nodes in the Data Flow Graph to unmarked.
 	 * Used in preparation for topological sort.
 	 */
	void clear_all_markings();

	/* Populates the given list with a sorted ordering of all the nodes in the graph.
	 * This order is a topologically sorted order.
	 * This is a valid ordering to visit the nodes when computing partial derivatives.
	 * This is the Topological Sorting Algorithm used:
	 * 
	 * 	sort(nodes):
	 *		clear_all_markings();
	 *		for all nodes:
	 *			if marked(node), continue.
	 *			if unmarked, visit(node).
	 *
	 *	visit(node):
	 *		mark temporarily
	 *		visit(child_one); visit(child_two);
	 *		mark permanently
	 *		add to head of list
	 */
	void top_sort(list<Node *> *sorted_nodes);
	void top_sort_visit(Node *node, list<Node *> *sorted_nodes);
};



#endif
