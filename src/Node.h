#ifndef NODE_H
#define NODE_H

#include <string>
#include <set>

#include "utilities.h"

using namespace std;


/* A node in the Data Flow Graph.
 * This node represents a variable (or a constant) in the computation.
 * Each node contains a name and a type (input, weight, intvar, constant, etc)
 * Some nodes (intvars, outputs, loss) may contain one or two children.
 * The output of a child node flows to its parent.
 * Non-constant nodes contain operation (add, mul, weight), that describes how their output is a function of their two children.
*/
 
class Node {

    string name;
    double constant_value;
    VariableType type;
    OperationType operation;

    set<Node *> *parents;
    set<string> *parent_names;

    string child_one_name, child_two_name;
    Node *child_one, *child_two;
    int num_children;

    /* The mark is used for topological sorting.
     * 0 means unmarked, 1 means temporary mark, 2 means permanent mark.
     */
    int mark;

public: 
    
    /* Basic Constructor.
     * Initializes NUM_CHILDREN and MARK to 0.
     * Initializes NAME, CHILD_ONE_NAME and CHILD_TWO_NAME to empty strings.
     * Initializes CONSTANT_VALUE to DBL_MIN.
     * Initializes TYPE and OPERATION to invalid.
     * Initializes CHILD_ONE and CHILD_TWO to NULL.
     * Initializes PARENTS AND PARENT_NAMES to an empty set.
     */
    Node();

    /* Constructor.
     * Creates a node with the given NAME.
     * If IS_CONSTANT is set, this node will represent a constant (a double).
     * Otherwise performs the same initializations as the default constructor.
     */
    Node(string node_name, bool is_constant);

    /* Destructor.
     * Does not delete the parent and children Node pointers, unless they are constant Nodes.
     * This is because the destructor for non-constant Nodes will eventually be called by the DFG destructor.
     */
     ~Node();


    /* -------------------- Getter and setter methods. ---------------------- */

    string get_name() const;
    void set_name(string new_name);

    VariableType get_type() const;
    /* Constant nodes cannot have their type set. */
    void set_type(VariableType new_type);
    bool is_constant() const;

    OperationType get_operation() const;
    /* Constant nodes cannot have their operation set. */
    void set_operation(OperationType new_operation);

    set<Node *> *get_parents() const;
    set<string> *get_parent_names() const;
    bool has_parent() const;

    /* Adds the given NEW_PARENT to the set of parents (and its name to the set of parent names).
     * 
     * Returns false if the given NEW_PARENT is NULL or if NEW_PARENT is a constant node.
     * Returns false if the given NEW_PARENT is an INPUT, WEIGHT or EXP_OUTPUT node.
     * Returns false if this node is a loss node, unless NEW_PARENT is the current node.
     *  (A loss node's parent is itself).
     *
     * Otherwise, return true. */
    bool add_parent(Node *new_parent);


    string get_child_one_name() const;
    string get_child_two_name() const;
    Node *get_child_one() const;                  
    Node *get_child_two() const;

    /* Sets a child of the current node to be the given node NEW_CHILD.
     * If the current node has no children, NEW_CHILD becomes Child 1.
     * If the current node has no children, NEW_CHILD becomes Child 2.
     * If the current node has two children already, this method returns false and nothing happens.
     * If the current node is constant, this method returns false and nothing happens.
     * If the current node is an INPUT, WEIGHT or EXP_OUTPUT node, returns false and nothing happens.
     * If the given NEW_CHILD is a loss node, returns false and nothing happens.
     * This method returns true otherwise.
     */
    bool set_child(Node *new_child);
    bool has_child_with_name(const string& child_name) const;
    int get_num_children() const;


    /* --------------- Helper Functions for Topological Sorting --------------- */

    /* Clearing the mark sets it to 0.
     * A temporary mark is 1, and a permanent mark is 2.
     */
    int get_mark() const;
    void clear_mark();
    void temporary_mark();
    void permanent_mark();
    bool is_unmarked() const;
    bool is_temporary_marked() const;
    bool is_permanent_marked() const;

};



#endif