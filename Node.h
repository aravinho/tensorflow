#ifndef NODE_H
#define NODE_H

#include <string>
//#include "Compiler.h"

using namespace std;

/* A node in the dependency graph.
 * Contains a name, operation (add, mul, weight)
 * Contains an array of partial derivatives, one w.r.t. each of the weight variables in the graph.
 * Contains pointers to its two operand nodes.
*/
class Node {
    char name[50];
    float constant_value;
    //enum variable_type_t type;
    char type[50];

    char parent_name[50];
    Node *parent;

    //enum operation_type_t operation;
    char operation[50];

    char child_one_name[50];
    char child_two_name[50];
    Node *child_one, *child_two;
    int num_children;

    /* The mark is used for topological sorting.
     * 0 means unmarked, 1 means temporary mark, 2 means permanent mark.
     */
    int mark;

public: 
    
    Node();
    Node(char *name, bool is_constant);
    //Node(float constant);

    char *get_name();
    void set_name(char *new_name);
    //enum variable_type_t get_type();
    char *get_type();
    void set_type(char *new_type);
    bool is_constant();

    char *get_parent_name();
    Node *get_parent();
    bool set_parent(Node *new_parent);

    //enum operation_type_t get_operation();
    char *get_operation();
    void set_operation(char *new_operation);

    char *get_child_one_name();
    char *get_child_two_name();
    Node *get_child_one();
    Node *get_child_two();
    bool set_child(Node *new_child);
    bool has_child_with_name(char *child_name);

    int get_num_children();

    int get_mark();
    void clear_mark();
    void temporary_mark();
    void permanent_mark();
    bool is_unmarked();
    bool is_temporary_marked();
    bool is_permanent_marked();

};



#endif