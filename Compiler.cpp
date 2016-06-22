#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include "Compiler.h"
#include "Node.h"

#define MAX_NUM_VARIABLES 100


using namespace std;

bool invalid_var_name(char *name);

bool invalid_var_name(char *name) {
    if (!name || strcmp(name, "") == 0) {
        return true;
    }
    return false;
}

enum instruction_type_t get_instruction_type(char inst[]) {
    if (strcmp(inst, "declare") == 0) {
        return DECLARE;
    }
    if (strcmp(inst, "define") == 0) {
        return DEFINE;
    } 
    
    return INVALID_INST;
}

enum variable_type_t get_variable_type(char var_type[]) {
    if (strcmp(var_type, "input") == 0) {
        return INPUT;
    }
    if (strcmp(var_type, "output") == 0) {
        return OUTPUT;
    }
    if (strcmp(var_type, "exp_output") == 0) {
        return EXP_OUTPUT;
    }
    if (strcmp(var_type, "weight") == 0) {
        return WEIGHT;
    }
    if (strcmp(var_type, "intvar") == 0) {
        return INTVAR;
    }
    if (strcmp(var_type, "loss") == 0) {
        return LOSS;
    }
   
    return INVALID_VAR_TYPE; 
}

enum operation_type_t get_operation_type(char oper[]) {
    if (strcmp(oper, "add") == 0) {
        return ADD;
    }
    if (strcmp(oper, "mul") == 0) {
        return MUL;
    }

    return INVALID_OPERATION;
}



Compiler::Compiler() {
    dfg = new DataFlowGraph();
}

/* Reads one line of code, and takes the appropriate actions with respect to the Data Flow Graph.
 * If the line is the declaration of a variable, a node is added to the graph.
 * If the line defines an expression for the variable, the respective node is updated.
 * Its operation is set, and the appropriate edges are added between nodes.
 * Once this method is called on every line, the Data Flow Graph is ready for the next step, the Topological sort.
 */
int parse_line(char line[], DataFlowGraph *dfg) {
    if (!line || strcmp(line, "") == 0) {
        return 0;
    }

    /* Grab the first token of the instruction (first token in the line).
     * Use this to determine what actions to take.
     */
    char *first_token = strtok(line, " ");
    instruction_type_t inst_type = get_instruction_type(first_token);
    if (inst_type == INVALID_INST) {
        return -1;
    }

    char *var_type;
    char *var_name;
    /* If it's a declaration (of an input, weight, intvar, etc),
     * simply create a new node, set the variable type, and add it to graph.
     */
    if (inst_type == DECLARE) {
        var_type = strtok(NULL, " ");
        if (get_variable_type(var_type) == INVALID_VAR_TYPE) {
            return -1;
        }

        var_name = strtok(NULL, " ");
        if (invalid_var_name(var_name)) {
            return -1;
        }

        Node *new_node = new Node(var_name, false);
        new_node->set_type(var_type);
        dfg->add_node(new_node);
        return 0;
    } 

    /* If the instruction is an expression that defines a variable:
     * Update the "operation" field of the variable's node to be "add" or "mul".
     * Create the two-way binding between the operands (children) and the current (parent) node. 
     */
    else if (inst_type == DEFINE) {
        var_name = strtok(NULL, " ");
        Node *node = dfg->get_node(var_name);

        char *equal_sign = strtok(NULL, " ");
        char *operation = strtok(NULL, " ");
        if (get_operation_type(operation) == INVALID_OPERATION) {
            return -1;
        }
        node->set_operation(operation);

        char *operand_1 = strtok(NULL, " ");
        char *operand_2 = strtok(NULL, " ");

        bool success = dfg->add_flow_edge(operand_1, var_name);
        success = success && dfg->add_flow_edge(operand_2, var_name);
        return success;
    }

    return -1;
    
}

void generate_partial_var_name(char dst[], char *var1, char *var2) {
    strcpy(dst, "d/");
    strcat(dst, var1);
    strcat(dst, "/d/");
    strcat(dst, var2);
}


void declare_partial_lambda(Node *node, char *loss_name, ofstream &gcp, char partial_var_name[]) {
    char line[100];
    strcpy(line, "declare ");
    if (strcmp(node->get_type(), "weight") == 0) {
        strcat(line, "output ");
    } else {
        strcat(line, "intvar ");
    }

    generate_partial_var_name(partial_var_name, loss_name, node->get_name());
    strcat(line, partial_var_name);
    cout << line << endl;
    gcp.write(line, strlen(line));
    gcp.write("\n", 1);
}

void define_partial_lambda(Node *node, char *loss_name, ofstream &gcp, char partial_var_name[]) {
    char line[100];
    strcpy(line, "define ");
    strcat(line, partial_var_name);
    strcat(line, " = ");

    /* partial(x, x) = 1 for any variable x.
     * This usually applied when the given NODE is the loss node.
     */
    if (strcmp(loss_name, node->get_name()) == 0) {
        //strcat(line, "1 1");
        strcat(line, "1");
    }

    else {
        char partial_lambda_parent[100], partial_parent_child[100];

        generate_partial_var_name(partial_lambda_parent, loss_name, node->get_parent_name());
        generate_partial_var_name(partial_parent_child, node->get_parent_name(), node->get_name());

        strcat(line, "mul ");
        strcat(line, partial_lambda_parent);
        strcat(line, " ");
        strcat(line, partial_parent_child);
    }

    

    cout << line << endl;
    gcp.write(line, strlen(line));
    gcp.write("\n", 1);

}

void declare_childrens_partial(Node *node, ofstream &gcp, char child_one_partial[], char child_two_partial[], int *declared_child_one_partial, int *declared_child_two_partial) {
    int num_children = node->get_num_children();

    char line_one[100];
    char line_two[100];

    if (num_children >= 1 && !node->get_child_one()->is_constant()) {
        strcpy(line_one, "declare intvar ");
        generate_partial_var_name(child_one_partial, node->get_name(), node->get_child_one_name());
        strcat(line_one, child_one_partial);
        cout << line_one << endl;
        gcp.write(line_one, strlen(line_one));
        gcp.write("\n", 1);
        *declared_child_one_partial = 1;
    }

    // if theres a second child, it's different from the first, and it's not constant
    if (num_children >= 2 && strcmp(node->get_child_one_name(), node->get_child_two_name()) != 0 && !node->get_child_two()->is_constant()) {
        strcpy(line_two, "declare intvar ");
        generate_partial_var_name(child_two_partial, node->get_name(), node->get_child_two_name());
        strcat(line_two, child_two_partial);
        cout << line_two << endl;
        gcp.write(line_two, strlen(line_two));
        gcp.write("\n", 1);
        *declared_child_two_partial = 1;
    }

}


void define_childrens_partial(Node *node, ofstream &gcp, char child_one_partial[], char child_two_partial[], int declared_child_one_partial, int declared_child_two_partial) {
    if (declared_child_one_partial) {
        char line_one[100];
        strcpy(line_one, "define ");
        strcat(line_one, child_one_partial);
        strcat(line_one, " = ");

        if (strcmp(node->get_operation(), "add") == 0) {
            strcat(line_one, "1");
        }
        else if (strcmp(node->get_operation(), "mul") == 0) {
            if (strcmp(node->get_child_one_name(), node->get_child_two_name()) == 0) {
                strcat(line_one, "mul 2 ");
                strcat(line_one, node->get_child_one_name());
            } else {
                strcat(line_one, node->get_child_two_name());
            }
        }

        cout << line_one << endl;
        gcp.write(line_one, strlen(line_one));
        gcp.write("\n", 1);
    }

    if (declared_child_two_partial) {
        char line_two[100];
        strcpy(line_two, "define ");
        strcat(line_two, child_two_partial);
        strcat(line_two, " = ");

        if (strcmp(node->get_operation(), "add") == 0) {
            strcat(line_two, "1");
        }
        else if (strcmp(node->get_operation(), "mul") == 0) {
            if (strcmp(node->get_child_one_name(), node->get_child_two_name()) == 0) {
                strcat(line_two, "mul 2 ");
                strcat(line_two, node->get_child_two_name());
            } else {
                strcat(line_two, node->get_child_one_name());
            }
        }

        cout << line_two << endl;
        gcp.write(line_two, strlen(line_two));
        gcp.write("\n", 1);
    }
}

void compile(char *shape_prog_filename, DataFlowGraph *dfg, char *gcp_filename) {
    ifstream shape_prog;
    shape_prog.open(shape_prog_filename);

    ofstream gcp;
    gcp.open(gcp_filename);

    char line[100];

    int i = 0;
    while(!shape_prog.eof())
    {
        shape_prog.getline(line, 100);
        gcp.write(line, strlen(line));
        gcp.write("\n", 1);
        parse_line(line, dfg);
        i++;
    }

    shape_prog.close();

    list<Node *> *top_sorted_nodes = new list<Node *>();
    dfg->top_sort(top_sorted_nodes);

    //cout << "Sorted order: " << endl;
    for (list<Node *>::iterator it = top_sorted_nodes->begin(); it != top_sorted_nodes->end(); ++it) {
        //cout << (*it)->get_name() << endl;

        char *loss_var_name = dfg->get_loss_var_name();

        char partial_lambda_var_name[100];

        // Checks if the current node is a child of the loss node 
        // for example, the diff node
        // this is to avoid double declarations and definitions of these partials, because the parent will already have defined them
        if (!dfg->get_loss_node()->has_child_with_name((*it)->get_name())) {
            declare_partial_lambda(*it, loss_var_name, gcp, partial_lambda_var_name);
            define_partial_lambda(*it, loss_var_name, gcp, partial_lambda_var_name);
        }
        
        
        char child_one_partial[100];
        char child_two_partial[100];
        int declared_child_one_partial = 0;
        int declared_child_two_partial = 0;
        declare_childrens_partial(*it, gcp, child_one_partial, child_two_partial, &declared_child_one_partial, &declared_child_two_partial);

        define_childrens_partial(*it, gcp, child_one_partial, child_two_partial, declared_child_one_partial, declared_child_two_partial);
        cout << endl;
        gcp.write("\n", 1);
        
    }

    gcp.close();

}

/*int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Must provide two arguments, a Shape Program file name and a GCP file name." << endl;
        return -1;
    }

    DataFlowGraph *dfg = new DataFlowGraph();
    compile("shape_prog.tf", dfg, "gcp.tf");
    cout << endl;
    cout << endl;
    cout << "num_nodes: " << dfg->get_num_nodes() << endl;

    unordered_map<string, Node*>* all_nodes = dfg->get_all_nodes();
    Node *node;
    for ( auto it = all_nodes->begin(); it != all_nodes->end(); ++it ) {
        cout << "name: " << it->first << endl;
        node = it->second;
        cout << "node name: " << node->get_name() << ", parent name: " << node->get_parent_name();
        cout << ", child one: " << node->get_child_one_name() << ", child two: " << node->get_child_two_name() << endl;
    }
    return 0;
}
*/
