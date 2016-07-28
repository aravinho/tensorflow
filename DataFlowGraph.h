#ifndef DATAFLOWGRAPH_H
#define DATAFLOWGRAPH_H

#include <unordered_map>
#include <list>

#include "Node.h"
#include "utilities.h"


using namespace std;


/* The Data Flow Graph is a graphical representation of how data flows through a computation.
 * The graph is represented as a mapping of node names to nodes, and is built during the parsing stage of the Compile Phase.
 * Each node represents a variable (or a constant) in the computation.
 * The Data Flow Graph is important, because it can be topologically sorted.
 * A topological sort of the DFG gives an order in which to visit nodes when computing partial derivatives.
 */
 
class DataFlowGraph {

	unordered_map<string, Node*>* nodes;
	int num_nodes;

	Node *loss_node;
	string loss_var_name;
	bool loss_node_added;
	

public:

	/* Constructor.
	 * Initializes the map of nodes, and set num_nodes to 0.
 	 */
	DataFlowGraph();

	/* Destructor.
	 * Frees each of the nodes in this graph.
	 * Frees the map of nodes itself.
	 */
	~DataFlowGraph();



	/* Adds a node to the Data Flow Graph.
 	 * Stores a mapping from node-name to node.
 	 * If the given node is the loss node, makes a note of this.
 	 * Returns 0 if the node was successfully added.
 	 * Returns -1 if there is already a node by this name in the graph.
 	 * Returns -1 if a loss node is being added for the second time.
 	 * Returns -1 if a the given node is constant.
 	 * In all failure cases, the new node is not added.
 	 */
	int add_node(Node *node);

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
	

	/* Returns the number of nodes in this Data Flow Graph.
 	 * Constant nodes are not included in this, so this number represents the number of variables in the computation.
 	 */
	int get_num_nodes() const;

	/* Returns the name of the loss node.
	 * This is an empty string if this DFG has not yet seen a loss node.
	 */
	string get_loss_var_name() const;

	/* Returns a pointer to the loss node.
	 * Returns a NULL pointer if this DFG has not yet seen a loss node.
	 */
	Node *get_loss_node() const;

	

	/* --------------------- Topological Sort Methods ------------------------- */



	/* Sets all nodes in the Data Flow Graph to unmarked.
 	 * Used in preparation for topological sort.
 	 */
	void clear_all_markings();

	/* Populates the given list with a sorted ordering of all the nodes in the graph.
	 * This order is a topologically sorted order.
	 * This is a valid ordering to visit the nodes when computing partial derivatives.
	 * This is the Topological Sorting Algorithm used:
	 * 
	 * 	top_sort(nodes):
	 *		clear_all_markings();
	 *		for all nodes:
	 *			if marked(node), continue.
	 *			if unmarked, top_sort_visit(node).
	 *
	 *	top_sort_visit(node):
	 *		mark temporarily
	 *		top_sort_visit(child_one); top_sort_visit(child_two);
	 *		mark permanently
	 *		add node to head of list
	 */
	void top_sort(list<Node *> *sorted_nodes);
	void top_sort_visit(Node *node, list<Node *> *sorted_nodes);
};



#endif
