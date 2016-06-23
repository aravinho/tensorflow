#ifndef NODE_H
#define NODE_H

#include <string>
#include "utilities.h"

using namespace std;

/* A node in the dependency graph.
 * Contains a name, operation (add, mul, weight)
 * Contains an array of partial derivatives, one w.r.t. each of the weight variables in the graph.
 * Contains pointers to its two operand nodes.
*/
class Node {
    string name;
    float constant_value;
    VariableType type;
    //char type[50];

    string parent_name;
    //char parent_name[50];
    Node *parent;

    OperationType operation;
    //char operation[50];

    string child_one_name, child_two_name;
    //char child_one_name[50];
    //char child_two_name[50];
    Node *child_one, *child_two;
    int num_children;

    /* The mark is used for topological sorting.
     * 0 means unmarked, 1 means temporary mark, 2 means permanent mark.
     */
    int mark;

public: 
    
    Node();
    Node(string node_name, bool is_constant);
    //Node(float constant);

    string get_name() const;
    void set_name(string new_name);
    VariableType get_type() const;
    //char *get_type();
    void set_type(VariableType new_type);       // can we pass in VariableType::INPUT, for example?
    bool is_constant() const;
    OperationType get_operation() const;
    void set_operation(OperationType new_operation);

    string get_parent_name() const;
    //char *get_parent_name();
    Node *get_parent() const;                   // return Node & or Node *
    bool set_parent(Node *new_parent);

    
    string get_child_one_name() const;
    string get_child_two_name() const;
    //char *get_child_one_name();
    //char *get_child_two_name();
    Node *get_child_one() const;                  // return Node ref or ptr?
    Node *get_child_two() const;

    /* Sets a child of the current node to be the given node NEW_CHILD.
     * If the current node has no children, NEW_CHILD becomes Child 1.
     * If the current node has no children, NEW_CHILD becomes Child 2.
     * If the current node has two children already, this method returns false and nothing happens.
     * This method returns true otherwise.
     */
    bool set_child(Node *new_child);                // pass Node ref or ptr?
    
    bool has_child_with_name(const string& child_name) const;
    int get_num_children() const;

    int get_mark() const;
    void clear_mark();
    void temporary_mark();
    void permanent_mark();
    bool is_unmarked() const;
    bool is_temporary_marked() const;
    bool is_permanent_marked() const;

};



#endif