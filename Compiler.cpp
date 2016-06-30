#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include "Compiler.h"
#include "utilities.h"
#include "Node.h"

// TO DO
// instead of straight copying lines from shape to gcp, make sure we alter types
// inputs, weights, exp outs all become inputs, rest become intvars
// when we come across a define line, make sure it's not the definition of an input, weight or exp_outp


using namespace std;


/* ---------------- Constructor --------------- */

Compiler::Compiler() {
    dfg = new DataFlowGraph();
}


/* ---------------- Main Methods -------------- */

int Compiler::compile(const string& shape_prog_filename, const string& gcp_filename) {

    if (invalid_file_name(shape_prog_filename)) {
        cerr << "Invalid Shape Program file name." << endl;
        return OTHER_ERROR;
    }

    // Open Shape Program and GCP file streams
    ifstream shape_prog(shape_prog_filename);
    ofstream gcp(gcp_filename);

    // buffer into which we read a line from the file
    char shape_line[MAX_LINE_LENGTH];

    // buffer to hold an identical copy of the line read in.
    // This line is mangled in the creation of the GCP-near-duplicate (See comment in Compiler.h)
    char shape_line_copy[MAX_LINE_LENGTH];

    // line to hold the GCP-near-duplicate line
    char gcp_line[MAX_LINE_LENGTH];

    // indicates whether the GCP-near-duplicate was created successfully
    int duplicate_success = 0;

    // indicates whether a line was successfully parsed
    int parse_success;

    // Iterate through all the lines of the Shape Program
    // Copy the GCP-near-duplicate line into the GCP, then send the line to be parsed.
    while(!shape_prog.eof())
    {
        shape_prog.getline(shape_line, MAX_LINE_LENGTH);
        strcpy(shape_line_copy, shape_line);

        duplicate_success = duplicate_line_for_gcp(shape_line_copy, gcp_line);

        if (duplicate_success == DUPLICATE_SUCCESS_DECLARE) {
            gcp.write(gcp_line, strlen(gcp_line));
        }
        else if(duplicate_success == DUPLICATE_SUCCESS_DEFINE) {
            gcp.write(shape_line, strlen(shape_line));
        }
        else if (duplicate_success == DUPLICATE_SUCCESS_EMPTY_LINE) {
            
        }
        else {
            cerr << "Invalid line: " << shape_line << endl;
            return duplicate_success;
        }
        
        gcp.write("\n", 1);

        parse_success = parse_line(shape_line);
        if (parse_success != 0) {
            cerr << "Invalid line: " << shape_line << endl;
            return parse_success;
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
        
        gcp.write("\n", 1);
        
    }

    gcp.close();
    return 0;

}


int Compiler::parse_line(char line[]) {

    if (!line) {
        return INVALID_LINE;
    }
    if (strcmp(line, "") == 0) {
        return 0;
    }

    // Grab the first token of the instruction (first token in the line).
    // Use this to determine what actions to take.
    char *first_token = strtok(line, " ");
    if (first_token == NULL) {
        return INVALID_LINE;
    }

    InstructionType inst_type = get_instruction_type(string(first_token));
    if (inst_type == InstructionType::INVALID_INST) {
        return INVALID_LINE;
    }

    char *v_name, *v_type;
    string var_name;
    VariableType var_type;

    // If the line is a declaration of a variable,
    // simply create a new node, set the variable type, and add it to graph.
    if (inst_type == InstructionType::DECLARE) {
        // grab the variable type
        v_type = strtok(NULL, " ");
        if (v_type == NULL) {
            return INVALID_LINE;
        }
        var_type = get_variable_type(string(v_type));
        if (var_type == VariableType::INVALID_VAR_TYPE) {
            return INVALID_LINE;
        }

        // grab the variable name
        v_name = strtok(NULL, " ");
        if (v_name == NULL) {
            return INVALID_LINE;
        }
        var_name = string(v_name);
        if (invalid_var_name(var_name)) {
            return INVALID_VAR_NAME;
        }

        Node *new_node = new Node(var_name, false);
        new_node->set_type(var_type);
        
        return dfg->add_node(new_node);
    } 

    // If the instruction is an expression that defines a variable:
    // Update the "operation" field of the variable's node to be "add" or "mul".
    // Create the two-way binding between the operands (children) and the current (parent) node. 
    else if (inst_type == InstructionType::DEFINE) {

        // grab the variable name
        v_name = strtok(NULL, " ");
        if (v_name == NULL) {
            return INVALID_LINE;
        }
        var_name = string(v_name);
        if (invalid_var_name(var_name)) {
            return INVALID_VAR_NAME;
        }


        // grab the node with this name
        Node *node = dfg->get_node(var_name);

        // grab the operation and set the node's operation
        char *equal_sign = strtok(NULL, " ");
        if (equal_sign == NULL) {
            return INVALID_LINE;
        }
        char *op = strtok(NULL, " ");
        if (op == NULL) {
            return INVALID_LINE;
        }
        OperationType operation = get_operation_type(string(op));
        if (operation == OperationType::INVALID_OPERATION) {
            return INVALID_LINE;
        }
        node->set_operation(operation);


        string operand_1 = string(strtok(NULL, " "));
        string operand_2 = string(strtok(NULL, " "));

        bool success = dfg->add_flow_edge(operand_1, var_name);
        success = success && dfg->add_flow_edge(operand_2, var_name);
        return success;
    }

    return INVALID_LINE;
    
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

int Compiler::duplicate_line_for_gcp(char shape_line[], char gcp_line[]) {
    
    if (shape_line == NULL || gcp_line == NULL) {
        cout << "MULL" << endl;
        return OTHER_ERROR;
    }
    if (strcmp(shape_line, "") == 0) {
        return DUPLICATE_SUCCESS_EMPTY_LINE;
    }

    cout << "shape line: " << shape_line << endl;
    // determine instruction type
    char *first_token = strtok(shape_line, " ");
    if (first_token == NULL) {
        return INVALID_LINE;
    }
    InstructionType inst_type = get_instruction_type(string(first_token));
    if (inst_type == InstructionType::INVALID_INST) {
        return INVALID_LINE;
    }


    // If define, make sure we're not defining an input, weight or exp_output
    if (inst_type == InstructionType::DEFINE) {
        
        // grab name
        char *v_name = strtok(NULL, " ");
        if (v_name == NULL) {
            return INVALID_VAR_NAME;
        }
        string var_name(v_name);

        // grab variable type from DFG
        // cannot define a variable without declaring it first
        Node *var_node = dfg->get_node(var_name);
        if (var_node == NULL) {
            return VAR_DEFINED_BEFORE_DECLARED;
        }
        VariableType var_type = var_node->get_type();

        // cannot define an input, weight or exp_output
        if (var_type == VariableType::INVALID_VAR_TYPE || var_type == VariableType::INPUT
            || var_type == VariableType::WEIGHT || var_type == VariableType::EXP_OUTPUT) {
            return CANNOT_DEFINE_I_W_EO;
        }

        // do more checks before returning
        return DUPLICATE_SUCCESS_DEFINE;
    }

    // If declare, change type appropriately
    if (inst_type == InstructionType::DECLARE) {
        cout << "A" << endl;
        char *v_type = strtok(NULL, " ");
        if (v_type == NULL) {
            return INVALID_LINE;
        }
        VariableType var_type = get_variable_type(string(v_type));

        if (var_type == VariableType::INVALID_VAR_TYPE) {
            return INVALID_LINE;
        }

        char gcp_var_type[50];
        if (var_type == VariableType::INPUT || var_type == VariableType::WEIGHT ||var_type == VariableType::EXP_OUTPUT) {
            strcpy(gcp_var_type, "input");
        } else {
            strcpy(gcp_var_type, "intvar");
        }
        cout << "B" << endl;
        char *v_name = strtok(NULL, " ");
        if (v_name == NULL) {
            return INVALID_VAR_NAME;
        }

        // The line should be over now
        if (strtok(NULL, " ") != NULL) {
            return INVALID_LINE;
        }
        cout << "C" << endl;
        strcpy(gcp_line, "declare ");
        strcat(gcp_line, gcp_var_type);
        strcat(gcp_line, " ");
        strcat(gcp_line, v_name);
        cout << "gcp line: " << gcp_line << endl;
        return DUPLICATE_SUCCESS_DECLARE;
        // null terminator?
    }

    return INVALID_LINE;
}
