#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <stdio.h>

//#include "Compiler.h"
#include "utilities.h"
#include "Preprocessor.h"
#include "Node.h"

// TO DO
// instead of straight copying lines from shape to gcp, make sure we alter types
// inputs, weights, exp outs all become inputs, rest become intvars
// when we come across a define line, make sure it's not the definition of an input, weight or exp_outp


using namespace std;


/* ---------------- Constructor --------------- */

Preprocessor::Preprocessor() {
    variables = new unordered_map<string, VariableType> ();
    vectors = new unordered_map<string, VariableType> ();
    vector_dimensions = new unordered_map<string, int> ();
    defined_variables = new unordered_set<string> ();
    macros = new unordered_map<string, struct macro> ();

    macros_done = false;
}


/* ---------------- Main Methods -------------- */

int Preprocessor::expand_program(const string& prog_filename, const string& expanded_prog_filename) {

    if (invalid_file_name(prog_filename)) {
        cerr << "Invalid Program file name: " << prog_filename << endl;
        return INVALID_FILE_NAME;
    }

    ifstream prog(prog_filename);
    ofstream exp_prog(expanded_prog_filename);

    // buffer into which we read a line from the file
    char prog_line[MAX_LINE_LENGTH];

    // array of buffers to hold the expanded lines
    char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH];

    // indicates how many lines were generated from a given line in the Shape Program.
    // 1 if no expansion was necessary, -1 if the Shape Program line was invalid.
    int num_lines_expanded;

    while(!prog.eof()) {

        prog.getline(prog_line, MAX_LINE_LENGTH);
        num_lines_expanded = expand_line(expanded_prog_lines, prog_line);

        if (num_lines_expanded < 0) return num_lines_expanded;

        for (int i = 0; i < num_lines_expanded; i++) {
            exp_prog.write(expanded_prog_lines[i], strlen(expanded_prog_lines[i]));
            exp_prog.write("\n", 1);
        }

        exp_prog.write("\n", 1);

    }

    prog.close();
    exp_prog.close();

    return 0;

}



int Preprocessor::expand_line(char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH], char prog_line[]) {

    // edge error cases
    if (prog_line == NULL || expanded_prog_lines == NULL) return OTHER_ERROR;
    if (strcmp(prog_line, "") == 0) return 0;


    // store a copy of the line, because it is about to be mangled by strtok
    char prog_line_copy[MAX_LINE_LENGTH];
    strcpy(prog_line_copy, prog_line);

    // tokenize the line
    char *tokens[MAX_NUM_TOKENS];
    int num_tokens = tokenize_line(prog_line, tokens, delimiters);
    if (num_tokens < 0) return INVALID_LINE;
    if (num_tokens == 0) return 0;

    // grab the instruction type, and act accordingly
    if (!is_valid_instruction(string(tokens[0]))) return INVALID_LINE;
    InstructionType inst_type = get_instruction_type(string(tokens[0]));


    // macro instructions need to be parsed and stored, but are not written to the expanded program
    // declare instructions need no expanding
    // define instructions need expanding if they involve macros or vector operations
    // declare_vector instructions always need expanding into their components

    if (inst_type == InstructionType::MACRO) {

        // ensure all macros are together at the top of the program
        if (macros_done) return MACROS_NOT_AT_TOP;

        // validate and parse the macro into a macro struct
        struct macro macro;
        int valid_macro_line = parse_macro_line(prog_line, &macro);

        // clear the variables and defined_variables maps
        // these maps are used temporarily during the parsing of a macro
        delete variables;
        variables = new unordered_map<string, VariableType> ();
        delete defined_variables;
        defined_variables = new unordered_set<string> ();

        // if the macro was erroneous, return an error code
        if (valid_macro_line < 0) return valid_macro_line;

        // store the macro struct
        macros->insert(make_pair(macro.name, macro));
        return 0;

    }

    macros_done = true;

    if (inst_type == InstructionType::DECLARE) {
        // verify the line is a valid DECLARE instruction
        int valid_declare_line = is_valid_declare_line(tokens, num_tokens);
        if (valid_declare_line < 0) return valid_declare_line;

        // copy the line directly. It needs no expanding
        strcpy(expanded_prog_lines[0], prog_line_copy);

        VariableType var_type = get_variable_type(string(tokens[1]));
        string var_name(tokens[2]);

        // record the type of this variable
        // if it's an input, weight or exp_output, mark it as defined
        variables->insert(make_pair(var_name, var_type));
        if (var_type == VariableType::INPUT || var_type == VariableType::WEIGHT || var_type == VariableType::EXP_OUTPUT)
            defined_variables->insert(var_name);

        return 1;
    }
    
    if (inst_type == InstructionType::DEFINE) {
        // verify the line is a valid DEFINE instruction
        int valid_define_line = is_valid_define_line(tokens, num_tokens);
        if (valid_define_line < 0) return valid_define_line;

        // expand the line
        int num_lines = expand_define_instruction(tokens, num_tokens, expanded_prog_lines);
        if (num_lines < 0) return num_lines;
        // if the line didn't need expanding, copy the line directly
        if (num_lines == 0) {
            strcpy(expanded_prog_lines[0], prog_line_copy);
            num_lines = 1;
        }


        // mark this variable as defined
        defined_variables->insert(string(tokens[1]));
        return num_lines;
    }
    
    if (inst_type == InstructionType::DECLARE_VECTOR) {
        // verify the line is a valid DECLARE_VECTOR instruction
        int valid_declare_vector_line = is_valid_declare_vector_line(tokens, num_tokens);
        if (valid_declare_vector_line < 0) return valid_declare_vector_line;

        // write the expanded component declarations into expanded_shape_lines
        // grab the size of the vector from the return value of expand_declare_vector_instruction
        int vector_size = expand_declare_vector_instruction(tokens, expanded_prog_lines);

        // record the type and dimension of this vector
        vectors->insert(make_pair(string(tokens[2]), get_variable_type(string(tokens[1]))));
        vector_dimensions->insert(make_pair(string(tokens[2]), stoi(string(tokens[3]))));

        // the number of expanded lines is precisely the dimension of the vector (one line per component)
        return vector_size;

    }

    return INVALID_LINE;

}


/* --------------------------- Helper Functions ---------------------------- */


int tokenize_line(char line[], char *tokens[MAX_NUM_TOKENS], const string& delim) {

    if (line == NULL || tokens == NULL) return OTHER_ERROR;
    if (strcmp(line, "") == 0) return 0;
    
    char *next_token;
    int num_tokens = 0;

    // grab the first token and copy it into the tokens array
    next_token = strtok(line, delim.c_str());
    if (!next_token) return INVALID_LINE;
    tokens[num_tokens] = new char[MAX_LINE_LENGTH];
    strcpy(tokens[num_tokens], next_token);
    num_tokens++;

    // grab all the subsequent tokens and copy them into the tokens array
    while ((next_token = strtok(NULL, delim.c_str())) != NULL) {
        tokens[num_tokens] = new char[MAX_LINE_LENGTH];
        strcpy(tokens[num_tokens], next_token);
        num_tokens++;
    }

    return num_tokens;
}


int Preprocessor::is_valid_declare_line(char *tokens[], int num_tokens) {
    if (!tokens) return OTHER_ERROR;
    if (num_tokens != 3) return INVALID_LINE;
    if (tokens[0] == NULL || tokens[1] == NULL || tokens[2] == NULL) return OTHER_ERROR;

    if (strcmp(tokens[0], "declare") != 0) return INVALID_LINE;
    if (get_variable_type(tokens[1]) == VariableType::INVALID_VAR_TYPE) return BAD_VAR_TYPE;
    if (!is_valid_var_name(string(tokens[2]))) return INVALID_VAR_NAME;
    if (variables->count(string(tokens[2])) != 0) return VAR_DECLARED_TWICE;

    return 0;
}


int Preprocessor::is_valid_declare_vector_line(char *tokens[], int num_tokens) {

    if (!tokens) return OTHER_ERROR;
    if (num_tokens != 4) return INVALID_LINE;
    if (tokens[0] == NULL || tokens[1] == NULL || tokens[2] == NULL || tokens[3] == NULL) return OTHER_ERROR;    

    if (strcmp(tokens[0], "declare_vector") != 0) return INVALID_LINE;
    if (get_variable_type(tokens[1]) == VariableType::INVALID_VAR_TYPE) return BAD_VAR_TYPE;
    if (!is_valid_var_name(string(tokens[2]))) return INVALID_VAR_NAME;
    if (vectors->count(string(tokens[2])) != 0 || variables->count(string(tokens[2])) != 0) return VAR_DECLARED_TWICE;
    if (!is_int(string(tokens[3])) || !is_valid_vector_size(stoi(string(tokens[3])))) return BAD_VECTOR_SIZE;

    return 0;

}

int Preprocessor::parse_macro_line(char macro_line[], struct macro *macro) {
    
    if (macro_line == NULL || macro == NULL) return OTHER_ERROR;

    // tokenize the large macro line into its sublines (separated by semi-colons)
    char *sub_macro_lines[MAX_EXPANSION_FACTOR];
    int num_sub_lines = tokenize_line(macro_line, sub_macro_lines, ";");
    if (num_sub_lines < 2) return INVALID_LINE;

    // make sure the first sub line is valid and parse it
    int first_line_valid = parse_macro_first_line(sub_macro_lines[0], macro);
    if (first_line_valid < 0) return first_line_valid;

    macro->lines = new vector<string>();
    macro->num_lines = 0;

    // validate and parse subsequent sub lines.
    int subsequent_line_valid;
    for (int i = 1; i < num_sub_lines; i++) {
        subsequent_line_valid = parse_macro_subsequent_line(sub_macro_lines[i], macro);
        if (subsequent_line_valid < 0) return subsequent_line_valid;
        macro->lines->push_back(string(sub_macro_lines[i]));
        macro->num_lines++;
    }

    return 0;
    
}


int Preprocessor::parse_macro_subsequent_line(char line[], struct macro *macro) {
    
    if (line == NULL || macro == NULL) return OTHER_ERROR;

    // make a copy of the line
    char line_copy[MAX_LINE_LENGTH];
    strcpy(line_copy, line);
    // tokenize
    char *tokens[MAX_LINE_LENGTH];
    int num_tokens = tokenize_line(line_copy, tokens, delimiters);
    
    // temporarily declare or define this variable
    if (is_valid_declare_line(tokens, num_tokens)) {
        variables->insert(make_pair(string(tokens[2]), get_variable_type(string(tokens[1]))));
        return 0;
    }

    if (is_valid_define_line(tokens, num_tokens)) {
        defined_variables->insert(string(tokens[1]));
        return 0;
    }

    return INVALID_LINE;
}

int Preprocessor::parse_macro_first_line(char first_line[], struct macro *macro) {

    if (first_line == NULL || macro == NULL) return OTHER_ERROR;

    // make a copy of the line
    char line_copy[MAX_LINE_LENGTH];
    strcpy(line_copy, first_line);
    // tokenize
    char *tokens[MAX_LINE_LENGTH];
    int num_tokens = tokenize_line(line_copy, tokens, delimiters);

    // check for trivial errors
    if (num_tokens < 5 || num_tokens > 6) return INVALID_LINE;
    if (tokens[0] == NULL || tokens[1] == NULL || tokens[2] == NULL ||
        tokens[3] == NULL || tokens[4] == NULL) return OTHER_ERROR; 
    if (num_tokens == 6 && tokens[5] == NULL) return OTHER_ERROR;

    // grab the tokens in strings
    string first_token(tokens[0]), second_token(tokens[1]), third_token(tokens[2]);
    string fourth_token(tokens[3]), fifth_token(tokens[4]);
    string sixth_token = "";

    // make sure the line format is correct
    if (first_token.compare("#macro") != 0) return INVALID_LINE;
    if (!is_valid_var_name(second_token)) return INVALID_VAR_NAME;
    if (third_token.compare("=") != 0) return INVALID_LINE;
    if (!is_valid_macro_name(fourth_token)) return INVALID_MACRO_NAME;
    if (!is_valid_var_name(fifth_token) || second_token.compare(fifth_token) == 0) return INVALID_LINE;
    if (num_tokens == 6) {
        sixth_token = tokens[5];
        if (!is_valid_var_name(sixth_token) || second_token.compare(sixth_token) == 0) return INVALID_LINE;
    }

    // populate the fields of the macro struct
    macro->name = fourth_token;
    macro->is_binary = (num_tokens == 6 ? true : false);
    macro->result = second_token;
    macro->operand1 = fifth_token;
    macro->operand2 = sixth_token;

    // declare the result and operand variables temporarily
    variables->insert(make_pair(macro->result, VariableType::INTVAR));
    variables->insert(make_pair(macro->operand1, VariableType::INTVAR));
    if (macro->is_binary) variables->insert(make_pair(macro->operand2, VariableType::INTVAR));

    return 0;

}


int Preprocessor::is_valid_define_line(char *tokens[], int num_tokens) {

    if (!tokens) return OTHER_ERROR;
    if (num_tokens < 4 || num_tokens > 6) return INVALID_LINE;
    if (tokens[0] == NULL || tokens[1] == NULL || tokens[2] == NULL || tokens[3] == NULL) return OTHER_ERROR;

    string first_token(tokens[0]);
    string second_token(tokens[1]);
    string third_token(tokens[2]);

    // every define instruction resembles "define <var_name> = ..."
    if (first_token.compare("define") != 0 || !is_valid_var_name(second_token) || third_token.compare("=") != 0)
        return INVALID_LINE;

    // input, weight and expected output variables cannot be defined
    VariableType var_type = get_variable_type(second_token);
    if (var_type == VariableType::INPUT || var_type == VariableType::WEIGHT || var_type == VariableType::EXP_OUTPUT)
        return CANNOT_DEFINE_I_W_EO;

    string fourth_token(tokens[3]);


    // a variable could be defined as a constant
    if (is_constant(fourth_token)) {
        // the variable being defined must have been declared, but cannot have been defined
        if ((variables->count(second_token) == 0 || defined_variables->count(second_token) != 0)) return VAR_DECLARED_TWICE;
        return (num_tokens == 4 ? 0 : INVALID_LINE);
    }


    // a variable could be defined as an operation of 1 or 2 constants/variables
    // this operation might be a primitive or might be a user defined macro
    if (is_valid_primitive(fourth_token) || is_valid_macro(fourth_token)) {

        // the variable being defined must have been declared, but cannot have been defined
        if ((variables->count(second_token) == 0 || defined_variables->count(second_token) != 0)) return VAR_DECLARED_TWICE;
        
        if (is_binary_primitive(fourth_token) || is_binary_macro(fourth_token)) {
            if (num_tokens != 6) return INVALID_LINE;

            string fifth_token(tokens[4]), sixth_token(tokens[5]);
            bool first_operand_constant = is_constant(fifth_token);
            bool second_operand_constant = is_constant(sixth_token);

            if (!first_operand_constant)
                if (variables->count(fifth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;

            if (!second_operand_constant)
                if (variables->count(sixth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;

            return 0;

        }

        else if (is_unary_primitive(fourth_token) || is_unary_macro(fourth_token)) {
            
            if (num_tokens != 5) return INVALID_LINE;

            string fifth_token(tokens[4]);
            bool first_operand_constant = is_constant(fifth_token);

            if (!first_operand_constant)
                if (variables->count(fifth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;

            return 0;

        }

    }


    // a variable could be defined as an operation of 2 vectors, a vector and a variable, or a vector and a constant
    if (is_valid_vector_operation(fourth_token)) {

        // the variable being defined must have been declared, but cannot have been defined
        if (get_operation_type(fourth_token) == OperationType::DOT) {
            if (variables->count(second_token) == 0 || defined_variables->count(second_token) != 0) return VAR_DECLARED_TWICE;
        } else {
            if ((vectors->count(second_token) == 0 || defined_variables->count(second_token) != 0)) return VAR_DECLARED_TWICE;
        }

        if (is_binary_vector_operation(fourth_token)) {
            
            if (num_tokens != 6) return INVALID_LINE;
            
            string fifth_token(tokens[4]), sixth_token(tokens[5]);

            if (vectors->count(fifth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
            if (vectors->count(sixth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
            if (vector_dimensions->at(fifth_token) != vector_dimensions->at(sixth_token) ||
                vector_dimensions->at(fifth_token) != vector_dimensions->at(second_token))
                return VECTORS_OF_DIFFERENT_DIMENSION;

            return 0;

        }        

        else if (is_unary_vector_operation(fourth_token)) {

            if (num_tokens != 6) return INVALID_LINE;

            string fifth_token(tokens[4]), sixth_token(tokens[5]);

            if (vectors->count(fifth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
            if (vector_dimensions->at(fifth_token) != vector_dimensions->at(second_token)) return VECTORS_OF_DIFFERENT_DIMENSION;
            if (!is_constant(sixth_token)) {
                if (variables->count(sixth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
            }
            return 0;
            
        }

    }


    // a variable could be defined as equivalent to another variable
    // this other variable must have been declared already
    if (is_valid_var_name(fourth_token)) {
        // the variable being defined must have been declared, but cannot have been defined
        if ((variables->count(second_token) == 0 || defined_variables->count(second_token) != 0)) return VAR_DECLARED_TWICE;

        if (num_tokens != 4) return INVALID_LINE;
        if (variables->count(fourth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
        return 0;
    }


    return INVALID_LINE;

}


int Preprocessor::expand_declare_vector_instruction(char *tokens[], char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]) {
    char *vec_type = tokens[1], *vec_name = tokens[2];
    int vec_size = stoi(tokens[3]);
        
    // copy expanded declaration lines into expanded_shape_lines
    for (int i = 0; i < vec_size; i++) {
        sprintf(expanded_prog_lines[i], "declare %s %s.%d", vec_type, vec_name, i);
    }

    return vec_size;    
}


int Preprocessor::expand_define_instruction(char *tokens[], int num_tokens, char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]) {

    string var_name(tokens[1]);
    string operation(tokens[3]);

    if (is_valid_vector_operation(operation)) {

        OperationType vec_oper = get_operation_type(operation);
        string operand1(tokens[4]);
        string operand2(tokens[5]);
        return expand_vector_instruction(vec_oper, var_name, operand1, operand2, expanded_prog_lines);

    }

    else if (is_valid_macro(operation)) {

        if (num_tokens == 5) {
            string operand(tokens[4]);
            return expand_unary_macro(operation, operand, var_name, expanded_prog_lines);
        }
        if (num_tokens == 6) {
            string operand1(tokens[4]);
            string operand2(tokens[5]);
            return expand_binary_macro(operation, operand1, operand2, var_name, expanded_prog_lines);
        }

    }

    return 0;

}


int Preprocessor::expand_unary_macro(const string& macro_name, const string& operand, const string& result, 
    char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]) {

    struct macro macro = macros->at(macro_name);
    vector<string> *macro_lines = macro.lines;

    for (int i = 0; i < macro.num_lines; i++) {
        string dummy_line = macro_lines->at(i);
        string modified_line = substitute_dummy_names(dummy_line, macro.result, macro.operand1, "", result, operand, "");
        strcpy(expanded_prog_lines[i], modified_line.c_str());
    }

    return macro.num_lines;

}


int Preprocessor::expand_binary_macro(const string& macro_name, const string& operand1, const string& operand2, const string& result, 
    char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]) {

    struct macro macro = macros->at(macro_name);
    vector<string> *macro_lines = macro.lines;

    for (int i = 0; i < macro.num_lines; i++) {
        string dummy_line = macro_lines->at(i);
        string modified_line = substitute_dummy_names(dummy_line, macro.result, macro.operand1, macro.operand2, result, operand1, operand2);
        strcpy(expanded_prog_lines[i], modified_line.c_str());
    }

    return macro.num_lines;

}



string Preprocessor::substitute_dummy_names(const string& dummy_line, string dummy_result, string dummy_op1, string dummy_op2,
    const string& result, const string& operand1, const string& operand2) {

    char *line_copy, *tokens[MAX_NUM_TOKENS];
    strcpy(line_copy, dummy_line.c_str());
    int num_tokens = tokenize_line(line_copy, tokens, delimiters);

    string modified_line = "";

    for (int i = 0; i < num_tokens; i++) {
        if (strcmp(tokens[i], dummy_result.c_str()) == 0) {
            modified_line.append(result + " ");
        }
        else if (strcmp(tokens[i], dummy_op1.c_str()) == 0) {
            modified_line.append(operand1 + " ");
        }
        else if (strcmp(tokens[i], dummy_op2.c_str()) == 0) {
            modified_line.append(operand2 + " ");
        }
        else {
            modified_line.append(tokens[i]);
        }
    }

    return modified_line;

}


int Preprocessor::expand_vector_instruction(const OperationType& oper_type, const string& result_vec, const string& operand1, const string& operand2,
    char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]) {

    int dimension = vector_dimensions->at(operand1);

    if (oper_type == OperationType::DOT)
        return expand_dot_product_instruction(result_vec, operand1, operand2, dimension, expanded_prog_lines);

    else if (oper_type == OperationType::COMPONENT_WISE_ADD)
        return expand_component_wise_add_instruction(result_vec, operand1, operand2, dimension, expanded_prog_lines);

    else if (oper_type == OperationType::COMPONENT_WISE_MUL)
        return expand_component_wise_mul_instruction(result_vec, operand1, operand2, dimension, expanded_prog_lines);

    else if (oper_type == OperationType::SCALE_VECTOR) 
        return expand_scale_vector_instruction(result_vec, operand1, operand2, dimension, expanded_prog_lines);

    else if (oper_type == OperationType::INCREMENT_VECTOR)
        return expand_increment_vector_instruction(result_vec, operand1, operand2, dimension, expanded_prog_lines);

    else return INVALID_LINE;
}


int Preprocessor::expand_dot_product_instruction(const string& result_vec, const string& vector1, const string& vector2, int dimension,
    char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]) {

    const char *vec1 = vector1.c_str(), *vec2 = vector2.c_str(), *res_vec = result_vec.c_str();

    // declare and define intvars for all the component-wise multiplications
    for (int i = 0; i < dimension; i++) {
        sprintf(expanded_prog_lines[2 * i], "declare intvar %s.%d", res_vec, i);
        sprintf(expanded_prog_lines[2 * i + 1], "define %s.%d = mul %s.%d %s.%d", res_vec, i, vec1, i, vec2, i);
    }

    // if the operand vectors' dimension is 1, the dot product is equal to the single component-wise product
    if (dimension == 1) {
        sprintf(expanded_prog_lines[2], "define %s = %s.0", res_vec, res_vec);
        return 3;
    }

    // accumulate the sum of all the component-wise products
    for (int j = 0; j < (dimension - 1); j++) {
        sprintf(expanded_prog_lines[2 * j + 2 * dimension], "declare intvar %s.%d", res_vec, dimension + j);
        if (j == 0) {
            sprintf(expanded_prog_lines[2 * j + 2 * dimension + 1], "define %s.%d = add %s.%d %s.%d", res_vec, dimension + j, res_vec, 0, res_vec, 1);
        } else {
            sprintf(expanded_prog_lines[2 * j + 2 * dimension + 1], "define %s.%d = add %s.%d %s.%d", res_vec, dimension + j, res_vec, dimension + j - 1, res_vec, j + 1);
        }
            
    }

    // define the value of the final dot product
    sprintf(expanded_prog_lines[4 * dimension - 2], "define %s = add %s.%d 0", res_vec, res_vec, 2 * dimension - 2);
    return (4 * dimension - 1);

}

int Preprocessor::expand_component_wise_add_instruction(const string& result_vec, const string& vec1, const string& vec2, int dimension, 
    char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]) {

    return 0;

}

int Preprocessor::expand_component_wise_mul_instruction(const string& result_vec, const string& vec1, const string& vec2, int dimension, 
    char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]) {

    return 0;
}

int Preprocessor::expand_scale_vector_instruction(const string& result_vec, const string& vec, const string& constant, int dimension, 
    char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]) {

    return 0;
}

int Preprocessor::expand_increment_vector_instruction(const string& result_vec, const string& vec, const string& constant, int dimension, 
    char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]) {

    return 0;
}


bool Preprocessor::is_valid_macro(const string& name) {
    return macros->count(name) > 0;
}

bool Preprocessor::is_binary_macro(const string& name) {
    return is_valid_macro(name) && macros->at(name).is_binary;
}

bool Preprocessor::is_unary_macro(const string& name) {
    return is_valid_macro(name) && macros->at(name).is_binary == false;
}


void test_macro_parsing_helper(char macro_line[]) {
    struct macro macro;

    Preprocessor p;
    int success = p.parse_macro_line(macro_line, &macro);
    if (success < 0) {
        cerr << "error" << endl;
        return;
    }

    cout << "macro name: " << macro.name << endl;    
    cout << "result: " << macro.result << ", operand1: " << macro.operand1 << ", operand2: " << macro.operand2 << endl;
    for (int i = 0; i < macro.num_lines; i++) {
        cout << macro.lines->at(i) << endl;
    }


}

void test_macro_parsing() {
    char a[1000], b[1000], c[1000], d[1000];
    strcpy(a, "#macro res = my_macro opone optwo; declare intvar lala; define lala = logistic opone; declare intvar q; define q = pow lala 3; define c = mul optwo q");
    test_macro_parsing_helper(a);

    strcpy(b, "#macro res = 1; define intvar z; define z = 3;");
    test_macro_parsing_helper(b);

    strcpy(c, "#define res = my_macro a b; res = mul a b");
    test_macro_parsing_helper(c);

    strcpy(d, "define res = my_macro a b; declare intvar q; define res = add q p;");
    test_macro_parsing_helper(d);
}

int main(int argc, char *argv[]) {
    test_macro_parsing();
    return 0;
}




