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


/* ---------------- Constructor/Destructor --------------- */


Compiler::Compiler() {
    dfg = new DataFlowGraph();
    visited_nodes = new unordered_set<Node *>();
    visited_node_names = new unordered_set<string>();
}


Compiler::~Compiler() {
    delete dfg;
    delete visited_nodes;
    delete visited_node_names;
}


/* ---------------- Main Methods -------------- */


int Compiler::compile(const string& shape_prog_filename, const string& gcp_filename) {

    if (!is_valid_file_name(shape_prog_filename)) {
        cerr << "Invalid Shape Program file name: " << shape_prog_filename << endl;
        return OTHER_ERROR;
    }

    // Open Shape Program and GCP file streams
    ifstream shape_prog(shape_prog_filename);
    ofstream gcp(gcp_filename);

    // buffer into which we read a line from the file
    string shape_line;

    // indicates whether the GCP-near-duplicate was created successfully
    int duplicate_success = 0;

    // indicates whether a line was successfully parsed
    int parse_success;

    // Iterate through all the lines of the Shape Program
    // Copy the GCP-near-duplicate line into the GCP, then send the line to be parsed.
    while(!shape_prog.eof())
    {
        getline(shape_prog, shape_line);

        duplicate_success = duplicate_line_for_gcp(shape_line, gcp);
        if (duplicate_success != 0) return duplicate_success;
        
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

    // Iterate through the sorted nodes
    // Define partial/loss/partial/current = partial/loss/partial/parent * partial/parent/partial/current
    // Define partial/current/partial/child using basic differentiation
    for (list<Node *>::iterator it = top_sorted_nodes->begin(); it != top_sorted_nodes->end(); ++it) {
        
        Node *curr_node = *it;
        
        string partial_var_name = declare_partial_lambda(curr_node, loss_node, gcp);
        define_partial_lambda(curr_node, loss_var_name, gcp, partial_var_name);

        string child_one_partial = declare_child_one_partial(curr_node, gcp);
        string child_two_partial = declare_child_two_partial(curr_node, gcp);
        define_child_one_partial(curr_node, gcp, child_one_partial);
        define_child_two_partial(curr_node, gcp, child_two_partial);

        visited_nodes->insert(curr_node);
        visited_node_names->insert(curr_node->get_name());
                
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
        if (var_type == VariableType::INVALID_VAR_TYPE) return BAD_VAR_TYPE;

        // grab the variable name
        var_name = tokens->at(2);
        if (!is_valid_expanded_var_name(var_name)) return INVALID_VAR_NAME;

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
        if (!is_valid_expanded_var_name(var_name)) return INVALID_VAR_NAME;

        // grab the node with this name
        Node *node = dfg->get_node(var_name);
        if (node == NULL) {
            return INVALID_LINE;
        }

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

        // if the variable v is being defined as equivalent to another variable y, define it as "add y 0"
        // we can be certain this will not create a cycle.
        // if y was a function of v, or y's operands were a function of v, then this means:
        // 1. v has already been defined, or 2. v is an input/weight/exp_output variable.
        // In either case, the current line is then invalid.
        // We trust the Preprocessor won't define variables in this cyclic manner.
        
        else if (is_valid_expanded_var_name(fourth_token)) {
            if (dfg->get_node(fourth_token) == NULL) return VAR_REFERENCED_BEFORE_DEFINED;
            node->set_operation(OperationType::ADD);
            success = dfg->add_flow_edge(fourth_token, var_name);
            success = success && dfg->add_flow_edge("0", var_name);
        }

        else if (is_unary_primitive(fourth_token)) {
            if (num_tokens != 5) return INVALID_LINE;
            node->set_operation(get_operation_type(fourth_token));
            success = dfg->add_flow_edge(tokens->at(4), var_name);
        }

        else if (is_binary_primitive(fourth_token)) {
            if (num_tokens != 6) return INVALID_LINE;
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
    if (var1 == "" || var2 == "") return "";
    return string("d/").append(var1).append("/d/").append(var2);
}

string generate_intvar_name(const string& var_name, int intvar_num) {
    if (var_name == "") return "";
    return string(var_name).append(":").append(to_string(intvar_num));
}


string Compiler::declare_partial_lambda(Node *node, Node *loss_node, ofstream& gcp) {
    
    if (!node || !loss_node || !gcp.is_open() || node->get_type() == VariableType::INVALID_VAR_TYPE) return "";

    // Checks if the current node is a child of the Loss node. 
    // Consider node x, a child of the Loss node.
    // The loss node will have alreay defined partial/loss/partial/x.
    // We must make sure x does not redefine this variable.
    if (loss_node->has_child_with_name(node->get_name())) return "";

    // We also check if the current node has a parent.
    // If not, then the loss node is independent of the current node.
    if (!node->has_parent()) {
        return "";
    }

    string line("declare ");
    if (node->get_type() == VariableType::WEIGHT) {
        line.append("output ");
    } else {
        line.append("intvar ");
    }

    string partial_name = generate_partial_var_name(loss_node->get_name(), node->get_name());
    line.append(partial_name);
    gcp << line << endl;
    return partial_name;
}


void Compiler::define_partial_lambda(Node *node, string loss_name, ofstream& gcp, string partial_var_name) {

    if (!node || loss_name == "" || !gcp.is_open() || partial_var_name == "") return;

    string line("define ");
    line.append(partial_var_name);
    line.append(" = ");

    // partial(x, x) = 1 for any variable x.
    // This usually applies when the given NODE is the loss node.
    if (loss_name.compare(node->get_name()) == 0) {
        line.append("1");
    }

    else {
        string visited_parent_name = "";
        set<string> *parent_names = node->get_parent_names();

        for (set<string>::iterator it = parent_names->begin(); it != parent_names->end(); ++it) {
            if (visited_node_names->count(*it) > 0) {
                visited_parent_name = *it;
                break;
            }
        }
        
        if (visited_parent_name == "") return;

        string partial_lambda_parent = generate_partial_var_name(loss_name, visited_parent_name);
        string partial_parent_child = generate_partial_var_name(visited_parent_name, node->get_name());

        line.append("mul ");
        line.append(partial_lambda_parent);
        line.append(" ");
        line.append(partial_parent_child);
    }

    gcp << line << endl;

}


string Compiler::declare_child_one_partial(Node *node, ofstream& gcp) {

    if (!node || !gcp.is_open()) return "";

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


string Compiler::declare_child_two_partial(Node *node, ofstream& gcp) {

    if (!node || !gcp.is_open()) return "";

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


void Compiler::define_child_one_partial(Node *node, ofstream& gcp, string child_one_partial) {

    if (!node || !gcp.is_open() || child_one_partial == "") return;

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
        intvars[0] = generate_intvar_name(child_one_partial, 0); intvars[1] = generate_intvar_name(child_one_partial, 1);
        intvars[2] = generate_intvar_name(child_one_partial, 2); intvars[3] = generate_intvar_name(child_one_partial, 3);
        for (int i = 0; i < 4; i++) gcp << "declare intvar " + intvars[i] << endl;

        // say f = logistic x
        gcp << "define " + intvars[0] + " = exp " + node->get_child_one_name() << endl;             // d/f/d/x_0 = e^x
        gcp << "define " + intvars[1] + " = add 1 " + intvars[0] << endl;                           // d/f/d/x_1 = 1 + d/f/d/x_0
        gcp << "define " + intvars[2] + " = pow " + intvars[1] + " 2" << endl;                      // d/f/d/x_2 = (d/f/d/x_1)^2
        gcp << "define " + intvars[3] + " = pow " + intvars[2] + " -1" << endl;                     // d/f/d/x_3 = 1 / d/f/d/x_2 

        gcp << "define " + child_one_partial + " = mul " + intvars[0] + " " + intvars[3] << endl;   // d/f/d/x = d/f/d/x_0 * d/f/d/x_3
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
        intvars[0] = generate_intvar_name(child_one_partial, 0); intvars[1] = generate_intvar_name(child_one_partial, 1);
        for (int i = 0; i < 2; i++) gcp << "declare intvar " + intvars[i] << endl;

        // say f = pow x y
        gcp << "define " + intvars[0] + " = add -1 " + node->get_child_two_name() << endl;                          // d/f/d/x_0 = y - 1 
        gcp << "define " + intvars[1] + " = pow " + node->get_child_one_name() + " " + intvars[0] << endl;       // d/f/d/x_1 = x ^ d/f/d/x_0

        gcp << "define " + child_one_partial + " = mul " + node->get_child_two_name() + " " + intvars[1] << endl;   // d/f/d/x = y * d/f/d/x_1

    }

}


void Compiler::define_child_two_partial(Node *node, ofstream& gcp, string child_two_partial) {
     
    if (!node || !gcp.is_open() || child_two_partial == "") return;

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
        intvars[0] = generate_intvar_name(child_two_partial, 0); intvars[1] = generate_intvar_name(child_two_partial, 1);
        for (int i = 0; i < 2; i++) gcp << "declare intvar " << intvars[i] << endl;

        // say f = pow x y
        gcp << "define " + intvars[0] + " = pow " + node->get_child_one_name() + " " + node->get_child_two_name() << endl;     // d/f/d/y_0 = x^y 
        gcp << "define " + intvars[1] + " = ln " + node->get_child_one_name() << endl;                                          // d/f/d/y_1 = ln(x)

        gcp << "define " + child_two_partial + " = mul " + intvars[0] + " " + intvars[1] << endl;   // d/f/d/y = d/f/d/y_0 * d/f/d/y_1

    }
}


int Compiler::duplicate_line_for_gcp(const string& shape_line, ofstream& gcp) {
    
    if (shape_line.compare("") == 0) return 0;
    if (!gcp.is_open()) return OTHER_ERROR;

    // tokenize the shape line
    vector<string> *tokens = new vector<string> ();
    int num_tokens = tokenize_line(shape_line, tokens, " ");
    if (num_tokens < 3) return INVALID_LINE;
    
    // determine instruction type
    InstructionType inst_type = get_instruction_type(tokens->at(0));
    if (inst_type == InstructionType::INVALID_INST) return INVALID_LINE;


    // If define, make sure we're not defining an input, weight or exp_output
    if (inst_type == InstructionType::DEFINE) {
        gcp << shape_line << endl;
        return 0;
    }

    // If declare, change type appropriately
    if (inst_type == InstructionType::DECLARE) {

        if (num_tokens != 3) return INVALID_LINE;
        VariableType var_type = get_variable_type(tokens->at(1));
        if (var_type == VariableType::INVALID_VAR_TYPE) return BAD_VAR_TYPE;

        string gcp_var_type;
        if (var_type == VariableType::INPUT || var_type == VariableType::WEIGHT ||var_type == VariableType::EXP_OUTPUT) {
            gcp_var_type = "input";
        } else {
            gcp_var_type = "intvar";
        }

        string var_name = tokens->at(2);
        gcp << "declare " << gcp_var_type << " " << var_name << endl;
        return 0;
    }

    return INVALID_LINE;
}

DataFlowGraph *Compiler::get_dfg() {
    return dfg;
}

unordered_set<Node *> *Compiler::get_visited_nodes() {
    return visited_nodes;
}

unordered_set<string> *Compiler::get_visited_node_names() {
    return visited_node_names;
}




/*int main(int argc, char *argv[]) {

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
*/