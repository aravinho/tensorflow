#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <stdio.h>
#include <vector>

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
    vector_dimensions = new unordered_map<string, int> ();

}


/* ---------------- Main Methods -------------- */


int Compiler::compile(const string& shape_prog_filename, const string& gcp_filename) {

    if (invalid_file_name(shape_prog_filename)) {
        cerr << "Invalid Shape Program file name: " << shape_prog_filename << endl;
        return OTHER_ERROR;
    }

    // Open Shape Program and GCP file streams
    ifstream shape_prog(shape_prog_filename);
    ofstream gcp(gcp_filename);

    // buffer into which we read a line from the file
    string shape_line;

    // line to hold the GCP-near-duplicate line
    string gcp_duplicate_line;

    // indicates whether the GCP-near-duplicate was created successfully
    int duplicate_success = 0;

    // indicates whether a line was successfully parsed
    int parse_success;

    // Iterate through all the lines of the Shape Program
    // Copy the GCP-near-duplicate line into the GCP, then send the line to be parsed.
    while(!shape_prog.eof())
    {
        getline(shape_prog, shape_line);
        cout << "shape line: " << shape_line << endl;
        //shape_prog.getline(shape_line, MAX_LINE_LENGTH);
        //strcpy(shape_line_copy, shape_line);

        duplicate_success = duplicate_line_for_gcp(shape_line, &gcp_duplicate_line);
        //duplicate_success = duplicate_line_for_gcp(shape_line_copy, gcp_line);

        if (duplicate_success == DUPLICATE_SUCCESS_DECLARE) {
            gcp << gcp_duplicate_line << endl;
            //gcp.write(gcp_line, strlen(gcp_line));
        }
        else if(duplicate_success == DUPLICATE_SUCCESS_DEFINE) {
            gcp << gcp_duplicate_line << endl;
            //gcp.write(shape_line, strlen(shape_line));
        }
        else if (duplicate_success == DUPLICATE_SUCCESS_EMPTY_LINE) {
            gcp << "\n";
        }
        else {
            cerr << "Invalid line: " << shape_line << endl;
            return duplicate_success;
        }
        
        parse_success = parse_line(shape_line);
        if (parse_success != 0) {
            cerr << "Couldn't be parsed: " << shape_line << endl;
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
    cout << "loss var name: " << loss_var_name << endl;

    // Iterate through the sorted nodes
    // Define partial/loss/partial/current = partial/loss/partial/parent * partial/parent/partial/current
    // Define partial/current/partial/child using basic differentiation
    for (list<Node *>::iterator it = top_sorted_nodes->begin(); it != top_sorted_nodes->end(); ++it) {
        
        Node *curr_node = *it;
        
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


int Compiler::parse_line(const string& line) {

    if (line.compare("") == 0) return 0;

    // tokenize the line
    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(line, tokens, " ");
    if (num_tokens < 3) return INVALID_LINE;

    // Grab the first token of the instruction (first token in the line).
    // Use this to determine what actions to take.
    InstructionType inst_type = get_instruction_type(tokens->at(0));
    if (inst_type == InstructionType::INVALID_INST) return INVALID_LINE;

    string var_name;
    VariableType var_type;

    // If the line is a declaration of a variable,
    // simply create a new node, set the variable type, and add it to graph.
    if (inst_type == InstructionType::DECLARE) {
        
        if (num_tokens != 3) return INVALID_LINE;
        // grab the variable type
        var_type = get_variable_type(tokens->at(1));
        if (var_type == VariableType::INVALID_VAR_TYPE) return INVALID_LINE;

        // grab the variable name
        var_name = tokens->at(2);
        if (!is_valid_var_name(var_name)) return INVALID_VAR_NAME;

        Node *new_node = new Node(var_name, false);
        new_node->set_type(var_type);
        
        return dfg->add_node(new_node);
    } 

    // If the instruction is an expression that defines a variable:
    // Update the "operation" field of the variable's node.
    // Create the two-way binding between the operands (children) and the current (parent) node. 
    else if (inst_type == InstructionType::DEFINE) {

        if (num_tokens < 4) return INVALID_LINE;

        // grab the variable name
        var_name = tokens->at(1);
        if (!is_valid_var_name(var_name)) return INVALID_VAR_NAME;

        // grab the node with this name
        Node *node = dfg->get_node(var_name);
        if (node == NULL) return INVALID_LINE;

        bool success = false;

        // grab the token after the equals sign
        // act based on the value of this token
        // A variable can defined as a constant, as equivalent to another variable,
        //  or as a function of one or two operands (the operands may be other variables or floats)
        string fourth_token = tokens->at(3);

        // if the variable is being defined as a constant c, define it as "add c 0"
        if (is_constant(fourth_token)) {
            node->set_operation(OperationType::ADD);
            success = dfg->add_flow_edge(fourth_token, var_name);
            success = success && dfg->add_flow_edge("0", var_name);
        }

        // if the variable is being defined as equivalent to another variable y, define it as "add y 0"
        // we can be certain this will not create a cycle.
        // if y was a function of x, or y's operands were a function of v, then this means:
        // 1. v has already been defined, or 2. v is an input/weight/exp_output variable.
        // In either case, the current line is then invalid.
        // We trust the Preprocessor won't define variables in this cyclic manner.
        
        else if (is_valid_var_name(fourth_token)) {
            if (dfg->get_node(fourth_token) == NULL) return VAR_REFERENCED_BEFORE_DEFINED;
            node->set_operation(OperationType::ADD);
            success = dfg->add_flow_edge(fourth_token, var_name);
            success = success && dfg->add_flow_edge("0", var_name);
        }

        else if (is_unary_primitive(fourth_token)) {
            if (num_tokens < 5) return INVALID_LINE;
            node->set_operation(get_operation_type(fourth_token));
            success = dfg->add_flow_edge(tokens->at(4), var_name);
        }

        else if (is_binary_primitive(fourth_token)) {
            if (num_tokens < 6) return INVALID_LINE;
            node->set_operation(get_operation_type(fourth_token));
            success = dfg->add_flow_edge(tokens->at(4), var_name);
            success = success && dfg->add_flow_edge(tokens->at(5), var_name);
        }

        else return INVALID_LINE;

        return success? 0 : INVALID_LINE; 

    }

    return INVALID_LINE;
    
}


/* -------------- Helper Functions ---------------- */

string generate_partial_var_name(const string& var1, const string& var2) {
    return string("d/").append(var1).append("/d/").append(var2);
}

string generate_intvar_name(const string& var_name, int intvar_num) {
    return string(var_name).append("_").append(to_string(intvar_num));
}


string declare_partial_lambda(Node *node, string loss_name, ofstream& gcp) {
    
    string line("declare ");
    if (node->get_type() == VariableType::WEIGHT) {
        line.append("output ");
    } else {
        line.append("intvar ");
    }

    string partial_name = generate_partial_var_name(loss_name, node->get_name());
    line.append(partial_name);
    gcp << line << endl;
    return partial_name;
}


void define_partial_lambda(Node *node, string loss_name, ofstream& gcp, string partial_var_name) {

    string line("define ");
    line.append(partial_var_name);
    line.append(" = ");

    // partial(x, x) = 1 for any variable x.
    // This usually applies when the given NODE is the loss node.
    if (loss_name.compare(node->get_name()) == 0) {
        line.append("1");
    }

    else {
        string partial_lambda_parent = generate_partial_var_name(loss_name, node->get_parent_name());
        string partial_parent_child = generate_partial_var_name(node->get_parent_name(), node->get_name());

        line.append("mul ");
        line.append(partial_lambda_parent);
        line.append(" ");
        line.append(partial_parent_child);
    }

    gcp << line << endl;

}


string declare_child_one_partial(Node *node, ofstream& gcp) {

    int num_children = node->get_num_children();
    string line;

    // make sure there is a first child and it's not a constant(float) node
    if (num_children >= 1 && !node->get_child_one()->is_constant()) {
        line = "declare intvar ";
        string child_one_partial = generate_partial_var_name(node->get_name(), node->get_child_one_name());
        line.append(child_one_partial);
        
        gcp << line << endl;
        return child_one_partial;
    }

    return "";

}


string declare_child_two_partial(Node *node, ofstream& gcp) {

    int num_children = node->get_num_children();
    string line;

    // make sure there is a second child, it's not a constant(float) node, and it's different from the first child
    if (num_children >= 2 && node->get_child_one_name().compare(node->get_child_two_name()) != 0 && !node->get_child_two()->is_constant()) {
        line = "declare intvar ";
        string child_two_partial = generate_partial_var_name(node->get_name(), node->get_child_two_name());
        line.append(child_two_partial);
        
        gcp << line << endl;        
        return child_two_partial;
    }

    return "";

}


void define_child_one_partial(Node *node, ofstream& gcp, string child_one_partial) {

    OperationType node_oper = node->get_operation();
    if (node_oper == OperationType::INVALID_OPERATION) return;

    // if c = a + b, partial(c, a) = 1
    if (node_oper == OperationType::ADD) {
        gcp << "define " + child_one_partial + " = 1" << endl;
    }

    else if (node_oper == OperationType::MUL) {

        // if c = a * a, partial(c, a) = 2a
        if (node->get_child_one_name().compare(node->get_child_two_name()) == 0) {
            gcp << "define " + child_one_partial + " = mul 2 " + node->get_child_one_name() << endl;
        } 
        // if c = a * b, where b != a, partial(c, a) = b
        else {
            gcp << "define " + child_one_partial + " = " + node->get_child_two_name() << endl;
        }
    } 

    // if c = logistic a, partial(c, a) = e^a / ((1 + e^a) ^ 2)
    else if (node_oper == OperationType::LOGISTIC) {

        string intvars[4];
        intvars[0] = generate_intvar_name(child_one_partial, 1); intvars[1] = generate_intvar_name(child_one_partial, 2);
        intvars[2] = generate_intvar_name(child_one_partial, 3); intvars[3] = generate_intvar_name(child_one_partial, 4);
        for (int i = 0; i < 4; i++) gcp << "declare intvar " + intvars[i] << endl;

        // say f = logistic x
        gcp << "define " + intvars[0] + " = exp " + node->get_child_one_name() << endl;             // d/f/d/x_1 = e^x
        gcp << "define " + intvars[1] + " = add 1 " + intvars[0] << endl;                           // d/f/d/x_2 = 1 + d/f/d/x_1
        gcp << "define " + intvars[2] + " = pow " + intvars[1] + " 2" << endl;                      // d/f/d/x_3 = (d/f/d/x_2)^2
        gcp << "define " + intvars[3] + " = pow " + intvars[2] + " -1" << endl;                     // d/f/d/x_4 = 1 / d/f/d/x_3 

        gcp << "define " + child_one_partial + " = mul " + intvars[0] + " " + intvars[3] << endl;   // d/f/d/x = d/f/d/x_1 * d/f/d/x_4
    }

    // if c = e^a, partial(c, a) = e^a
    else if (node_oper == OperationType::EXP) {
        gcp << "define " + child_one_partial + " = exp " + node->get_child_one_name() << endl;
    }

    // if c = ln a, partial(c, a) = 1/a
    else if (node_oper == OperationType::LN) {
        gcp << "define " + child_one_partial + " = pow " + node->get_child_one_name() + " -1" << endl;
    }

    // if c = a^b, partial(c, a) = b * a^(b - 1)
    else if (node_oper == OperationType::POW) {
        string intvars[2];
        intvars[0] = generate_intvar_name(child_one_partial, 1); intvars[1] = generate_intvar_name(child_one_partial, 2);
        for (int i = 0; i < 2; i++) gcp << "declare intvar " + intvars[i] << endl;

        // say f = pow x y
        gcp << "define " + intvars[0] + " = add -1 " + node->get_child_two_name() << endl;                          // d/f/d/x_1 = y - 1 
        gcp << "define " + intvars[1] + " = pow " + node->get_child_one_name() + " " + intvars[0] << endl;       // d/f/d/x_2 = x ^ d/f/d/x_1

        gcp << "define " + child_one_partial + " = mul " + node->get_child_two_name() + " " + intvars[1] << endl;   // d/f/d/x = y * d/f/d/x_2

    }

}


void define_child_two_partial(Node *node, ofstream& gcp, string child_two_partial) {
     
    OperationType node_oper = node->get_operation();
    if (node_oper == OperationType::INVALID_OPERATION) return;

    // if c = a + b, partial(c, b) = 1
    if (node_oper == OperationType::ADD) {
        gcp << "define " + child_two_partial + " = 1" << endl;
    }

    else if (node_oper == OperationType::MUL) {

        // if c = b * b, partial(c, b) = 2b
        if (node->get_child_one_name().compare(node->get_child_two_name()) == 0) {
            gcp << "define " + child_two_partial + " = mul 2 " + node->get_child_two_name() << endl;
        } 
        // if c = a * b, where b != a, partial(c, b) = a
        else {
            gcp << "define " + child_two_partial + " = " + node->get_child_one_name() << endl;
        }
    } 

    // if c = a^b, partial(c, b) = a^b * ln(a)
    else if (node_oper == OperationType::POW) {
        string intvars[2];
        intvars[0] = generate_intvar_name(child_two_partial, 1); intvars[1] = generate_intvar_name(child_two_partial, 2);
        for (int i = 0; i < 2; i++) gcp << "declare intvar " << intvars[i] << endl;

        // say f = pow x y
        gcp << "define " + intvars[0] + " = pow " + node->get_child_one_name() + " " + node->get_child_two_name() << endl;     // d/f/d/y_1 = x^y 
        gcp << "define " + intvars[1] + " = ln " + node->get_child_one_name() << endl;                                          // d/f/d/y_2 = ln(x)

        gcp << "define " + child_two_partial + " = mul " + intvars[0] + " " + intvars[1] << endl;   // d/f/d/y = d/f/d/y_1 * d/f/d/y_2

    }
}


int Compiler::duplicate_line_for_gcp(const string& shape_line, string *gcp_duplicate_line) {
    
    if (gcp_duplicate_line == NULL) return OTHER_ERROR;
    if (shape_line.compare("") == 0) return DUPLICATE_SUCCESS_EMPTY_LINE;

    // tokenize the shape line
    vector<string> *tokens = new vector<string> ();
    int num_tokens = tokenize_line(shape_line, tokens, " ");
    if (num_tokens < 3) return INVALID_LINE;
    
    // determine instruction type
    InstructionType inst_type = get_instruction_type(tokens->at(0));
    if (inst_type == InstructionType::INVALID_INST) return INVALID_LINE;


    // If define, make sure we're not defining an input, weight or exp_output
    if (inst_type == InstructionType::DEFINE) {
        *gcp_duplicate_line = shape_line;
        return DUPLICATE_SUCCESS_DEFINE;
    }

    // If declare, change type appropriately
    if (inst_type == InstructionType::DECLARE) {

        if (num_tokens != 3) return INVALID_LINE;
        VariableType var_type = get_variable_type(tokens->at(1));
        if (var_type == VariableType::INVALID_VAR_TYPE) return INVALID_LINE;

        string gcp_var_type;
        if (var_type == VariableType::INPUT || var_type == VariableType::WEIGHT ||var_type == VariableType::EXP_OUTPUT) {
            gcp_var_type = "input";
        } else {
            gcp_var_type = "intvar";
        }

        string var_name = tokens->at(2);
        
        *gcp_duplicate_line = "declare ";
        gcp_duplicate_line->append(gcp_var_type + " ");
        gcp_duplicate_line->append(var_name);
        return DUPLICATE_SUCCESS_DECLARE;
    }

    return INVALID_LINE;
}





int main(int argc, char *argv[]) {

    Node *a = new Node("a", false); 
    Node *b = new Node("-7", true);
    Node *c = new Node("c", false);
    c->set_operation(OperationType::LN);
    c->set_child(a);
    //c->set_child(b);

    ofstream gcp("foo.tf");

    string partial1 = declare_child_one_partial(c, gcp);
    string partial2 = declare_child_two_partial(c, gcp);
    if (partial1.compare("") != 0) define_child_one_partial(c, gcp, partial1);
    if (partial2.compare("") != 0) define_child_two_partial(c, gcp, partial2);

    
}
