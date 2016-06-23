#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include "Compiler.h"
#include "utilities.h"
#include "Node.h"

#define MAX_NUM_VARIABLES 100
#define MAX_LINE_LENGTH 1000
#define MAX_SHAPE_PROG_VAR_NAME_LENGTH 80
#define MAX_GCP_VAR_NAME_LENGTH 200


using namespace std;


/* ---------------- Constructor --------------- */

Compiler::Compiler() {
    dfg = new DataFlowGraph();
}


/* ---------------- Main Methods -------------- */

int Compiler::compile(const string& shape_prog_filename, const string& gcp_filename) {

    if (invalid_file_name(shape_prog_filename)) {
        cerr << "Invalid Shape Program file name." << endl;
        return -1;
    }

    // Open Shape Program and GCP file streams
    ifstream shape_prog(shape_prog_filename);
    ofstream gcp(gcp_filename);

    // buffer to hold the line currently being parsed
    char line[MAX_LINE_LENGTH];
    // indicates whether a line was successfully parsed
    int parse_success;

    // Iterate through all the lines of the Shape Program
    // Copy the line into the GCP, then send the line to be parsed.
    while(!shape_prog.eof())
    {
        shape_prog.getline(line, MAX_LINE_LENGTH);
        gcp.write(line, strlen(line));
        gcp.write("\n", 1);
        parse_success = parse_line(line);
        if (parse_success == -1) {
            cerr << "Invalid line: " << line << endl;
            return -1;
        }
    }

    shape_prog.close();



    // After the while loop, the Data Flow Graph is assembled.
    // Topologically sort the nodes of the Data Flow Graph.
    list<Node *> *top_sorted_nodes = new list<Node *>();
    dfg->top_sort(top_sorted_nodes);



    // Grab the Loss node
    Node *loss_node = dfg->get_loss_node();
    string loss_var_name = dfg->get_loss_var_name();

    // Iterate through the sorted nodes
    // Define partial/loss/partial/current = partial/loss/partial/parent * partial/parent/partial/current
    // Define partial/current/partial/child using basic differentiation
    for (list<Node *>::iterator it = top_sorted_nodes->begin(); it != top_sorted_nodes->end(); ++it) {
        Node *curr_node = *it;
        char partial_lambda_var_name[MAX_GCP_VAR_NAME_LENGTH];

        // Checks if the current node is a child of the Loss node. 
        // Consider node x, a child of the Loss node.
        // The loss node will have alreay defined partial/loss/partial/x.
        // We must make sure x does not redefine this variable.
        if (!loss_node->has_child_with_name(curr_node->get_name())) {
            string partial_var_name = declare_partial_lambda(curr_node, loss_var_name, gcp);
            define_partial_lambda(curr_node, loss_var_name, gcp, partial_var_name);
        }
        
         
        string child_one_partial = declare_child_one_partial(curr_node, gcp);
        string child_two_partial = declare_child_two_partial(curr_node, gcp);

        if (child_one_partial.compare("") != 0) {
            define_child_one_partial(curr_node, gcp, child_one_partial);
        }
        if (child_two_partial.compare("") != 0) {
            define_child_two_partial(curr_node, gcp, child_two_partial);
        }
        //declare_childrens_partial(curr_node, gcp, child_one_partial, child_two_partial, &declared_child_one_partial, &declared_child_two_partial);
        //define_childrens_partial(curr_node, gcp, child_one_partial, child_two_partial, declared_child_one_partial, declared_child_two_partial);
        
        gcp.write("\n", 1);
        
    }

    gcp.close();
    return 0;

}


int Compiler::parse_line(char line[]) {

    if (!line) {
        return -1;
    }
    if (strcmp(line, "") == 0) {
        return 0;
    }

    // Grab the first token of the instruction (first token in the line).
    // Use this to determine what actions to take.
    char *first_token = strtok(line, " ");
    InstructionType inst_type = get_instruction_type(string(first_token));
    if (inst_type == InstructionType::INVALID_INST) {
        return -1;
    }


    string var_type, var_name;
    // If the line is a declaration of a variable,
    // simply create a new node, set the variable type, and add it to graph.
    if (inst_type == InstructionType::DECLARE) {
        var_type = string(strtok(NULL, " "));
        if (get_variable_type(var_type) == VariableType::INVALID_VAR_TYPE) {
            return -1;
        }

        var_name = string(strtok(NULL, " "));
        if (invalid_var_name(var_name)) {
            return -1;
        }

        Node *new_node = new Node(var_name, false);
        new_node->set_type(get_variable_type(var_type));
        
        return dfg->add_node(new_node);
    } 

    // If the instruction is an expression that defines a variable:
    // Update the "operation" field of the variable's node to be "add" or "mul".
    // Create the two-way binding between the operands (children) and the current (parent) node. 
    else if (inst_type == InstructionType::DEFINE) {
        var_name = string(strtok(NULL, " "));
        Node *node = dfg->get_node(var_name);

        char *equal_sign = strtok(NULL, " ");
        string operation = string(strtok(NULL, " "));
        if (get_operation_type(operation) == OperationType::INVALID_OPERATION) {
            return -1;
        }
        node->set_operation(get_operation_type(operation));

        string operand_1 = string(strtok(NULL, " "));
        string operand_2 = string(strtok(NULL, " "));

        bool success = dfg->add_flow_edge(operand_1, var_name);
        success = success && dfg->add_flow_edge(operand_2, var_name);
        return success;
    }

    return -1;
    
}


/* -------------- Helper Functions ---------------- */

string generate_partial_var_name(const string& var1, const string& var2) {
    return string("d/").append(var1).append("/d/").append(var2);
}


string declare_partial_lambda(Node *node, string loss_name, ofstream &gcp) {
    char line[MAX_LINE_LENGTH];
    strcpy(line, "declare ");
    if (node->get_type() == VariableType::WEIGHT) {
        strcat(line, "output ");
    } else {
        strcat(line, "intvar ");
    }

    string partial_name = generate_partial_var_name(loss_name, node->get_name());
    strcat(line, partial_name.c_str());
    gcp.write(line, strlen(line));
    gcp.write("\n", 1);

    return partial_name;
}


void define_partial_lambda(Node *node, string loss_name, ofstream &gcp, string partial_var_name) {
    char line[MAX_LINE_LENGTH];
    strcpy(line, "define ");
    strcat(line, partial_var_name.c_str());
    strcat(line, " = ");

    // partial(x, x) = 1 for any variable x.
    // This usually applied when the given NODE is the loss node.
    if (loss_name.compare(node->get_name()) == 0) {
        strcat(line, "1");
    }

    else {
        string partial_lambda_parent = generate_partial_var_name(loss_name, node->get_parent_name());
        string partial_parent_child = generate_partial_var_name(node->get_parent_name(), node->get_name());

        strcat(line, "mul ");
        strcat(line, partial_lambda_parent.c_str());
        strcat(line, " ");
        strcat(line, partial_parent_child.c_str());
    }

    gcp.write(line, strlen(line));
    gcp.write("\n", 1);

}


string declare_child_one_partial(Node *node, ofstream &gcp) {
    int num_children = node->get_num_children();
    char line[MAX_LINE_LENGTH];

    // make sure there is a first child and it's not a constant(float) node
    if (num_children >= 1 && !node->get_child_one()->is_constant()) {
        strcpy(line, "declare intvar ");
        string child_one_partial = generate_partial_var_name(node->get_name(), node->get_child_one_name());
        strcat(line, child_one_partial.c_str());
        
        gcp.write(line, strlen(line));
        gcp.write("\n", 1);
        
        return child_one_partial;
    }

    return "";

}


string declare_child_two_partial(Node *node, ofstream &gcp) {
    int num_children = node->get_num_children();
    char line[MAX_LINE_LENGTH];

    // make sure there is a second child, it's not a constant(float) node, and it's different from the first child
    if (num_children >= 2 && node->get_child_one_name().compare(node->get_child_two_name()) != 0 && !node->get_child_two()->is_constant()) {
        strcpy(line, "declare intvar ");
        string child_two_partial = generate_partial_var_name(node->get_name(), node->get_child_two_name());
        strcat(line, child_two_partial.c_str());
        
        gcp.write(line, strlen(line));
        gcp.write("\n", 1);
        
        return child_two_partial;
    }

    return "";

}


void define_child_one_partial(Node *node, ofstream &gcp, string child_one_partial) {
    char line[MAX_LINE_LENGTH];
    strcpy(line, "define ");
    strcat(line, child_one_partial.c_str());
    strcat(line, " = ");

    // if c = a + b, partial(c, a) = 1
    if (node->get_operation() == OperationType::ADD) {
        strcat(line, "1");
    }
    else if (node->get_operation() == OperationType::MUL) {
        // if c = a * a, partial(c, a) = 2a
        if (node->get_child_one_name().compare(node->get_child_two_name()) == 0) {
            strcat(line, "mul 2 ");
            strcat(line, node->get_child_one_name().c_str());
        } 
        // if c = a * b, where b != a, partial(c, a) = b
        else {
            strcat(line, node->get_child_two_name().c_str());
        }
    }

    gcp.write(line, strlen(line));
    gcp.write("\n", 1);
}


void define_child_two_partial(Node *node, ofstream &gcp, string child_two_partial) {
    char line[MAX_LINE_LENGTH];
    strcpy(line, "define ");
    strcat(line, child_two_partial.c_str());
    strcat(line, " = ");

    // if c = a + b, partial(c, b) = 1
    if (node->get_operation() == OperationType::ADD) {
        strcat(line, "1");
    }
    else if (node->get_operation() == OperationType::MUL) {
        // if c = b * b, partial(c, b) = 2b
        if (node->get_child_one_name().compare(node->get_child_two_name()) == 0) {
            strcat(line, "mul 2");
            strcat(line, node->get_child_two_name().c_str());
        } 
        // if c = a * b, where b != a, partial(c, b) = a
        else {
            strcat(line, node->get_child_one_name().c_str());
        }
    }

    gcp.write(line, strlen(line));
    gcp.write("\n", 1);
}
