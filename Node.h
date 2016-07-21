#ifndef NODE_H
#define NODE_H

#include <string>
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
    float constant_value;
    VariableType type;
    OperationType operation;

    string parent_name;
    Node *parent;

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
     */
    Node();

    /* Constructor.
     * Creates a node with the given NAME.
     * If IS_CONSTANT is set, this node will represent a constant (a float).
     * Initializes NUM_CHILDREN and MARK to 0.
     */
    Node(string node_name, bool is_constant);


    /* -------------------- Getter and setter methods. ---------------------- */

    string get_name() const;
    void set_name(string new_name);

    VariableType get_type() const;
    void set_type(VariableType new_type);
    bool is_constant() const;

    OperationType get_operation() const;
    void set_operation(OperationType new_operation);

    string get_parent_name() const;
    Node *get_parent() const;

    /* Returns false if the given NEW_PARENT is NULL, and true otherwise. */
    bool set_parent(Node *new_parent);


    string get_child_one_name() const;
    string get_child_two_name() const;
    Node *get_child_one() const;                  
    Node *get_child_two() const;

    /* Sets a child of the current node to be the given node NEW_CHILD.
     * If the current node has no children, NEW_CHILD becomes Child 1.
     * If the current node has no children, NEW_CHILD becomes Child 2.
     * If the current node has two children already, this method returns false and nothing happens.
     * This method returns true otherwise.
     */
    bool set_child(Node *new_child);
    bool has_child_with_name(const string& child_name) const;
    int get_num_children() const;


    /* --------------- Helper Functions for Topological Sorting --------------- */


    int get_mark() const;
    void clear_mark();
    void temporary_mark();
    void permanent_mark();
    bool is_unmarked() const;
    bool is_temporary_marked() const;
    bool is_permanent_marked() const;

};



#endif