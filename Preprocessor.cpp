#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

#include "utilities.h"
#include "Preprocessor.h"


using namespace std;


/* ---------------- Constructor --------------- */

Preprocessor::Preprocessor() {
    variables = new unordered_map<string, VariableType> ();
    vectors = new unordered_map<string, VariableType> ();
    vector_dimensions = new unordered_map<string, int> ();
    defined_variables = new unordered_set<string> ();
    macros = new unordered_map<string, struct macro*> ();

    macros_done = false;
}


/* ------------------------------- Main Methods ---------------------------- */

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
   vector<string> *expanded_prog_lines;

    // indicates how many lines were generated from a given line in the Shape Program.
    // 1 if no expansion was necessary, -1 if the Shape Program line was invalid.
    int num_lines_expanded;

    while(!prog.eof()) {

        prog.getline(prog_line, MAX_LINE_LENGTH);
        expanded_prog_lines = new vector<string>();
        num_lines_expanded = expand_line(expanded_prog_lines, prog_line);

        if (num_lines_expanded < 0) return num_lines_expanded;

        for (int i = 0; i < num_lines_expanded; i++) {
            exp_prog.write(expanded_prog_lines->at(i).c_str(), expanded_prog_lines->at(i).size());
            exp_prog.write("\n", 1);
        }

        exp_prog.write("\n", 1);

        delete expanded_prog_lines;

    }

    prog.close();
    exp_prog.close();

    return 0;

}


int Preprocessor::expand_line(vector<string> *expanded_prog_lines, char prog_line[]) {
    
    // edge error cases
    if (prog_line == NULL || expanded_prog_lines == NULL) return OTHER_ERROR;
    if (strcmp(prog_line, "") == 0) return 0;

    // store a copy of the line, because it is about to be mangled by strtok
    char *prog_line_copy = new char[MAX_LINE_LENGTH];
    strcpy(prog_line_copy, prog_line);

    // tokenize the line
    char *tokens[MAX_NUM_TOKENS];
    int num_tokens = tokenize_line(prog_line_copy, tokens, delimiters);
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
        struct macro *macro = new struct macro();
        int valid_macro_line = parse_macro_line(prog_line, macro);

        // clear the variables and defined_variables maps
        // these maps are used temporarily during the parsing of a macro
        delete variables;
        variables = new unordered_map<string, VariableType> ();
        delete defined_variables;
        defined_variables = new unordered_set<string> ();

        // if the macro was erroneous, return an error code
        if (valid_macro_line < 0) return valid_macro_line;

        // store the macro struct
        macros->insert(make_pair(macro->name, macro));

        return 0;

    }
    
    macros_done = true;

    if (inst_type == InstructionType::DECLARE) {
        // verify the line is a valid DECLARE instruction
        int valid_declare_line = is_valid_declare_line(tokens, num_tokens);
        if (valid_declare_line < 0) return valid_declare_line;

        // copy the line directly. It needs no expanding
        expanded_prog_lines->push_back(string(prog_line));
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
            expanded_prog_lines->push_back(prog_line);
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


/* --------------------------- Tokenizer ---------------------------- */


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

/* ------------------------- Main Expansion Methods ------------------------- */


int Preprocessor::expand_declare_vector_instruction(char *tokens[], vector<string> *expanded_prog_lines) {
    char *vec_type = tokens[1], *vec_name = tokens[2];
    int vec_size = stoi(tokens[3]);
    // copy expanded declaration lines into expanded_shape_lines
    char *buffer;
    for (int i = 0; i < vec_size; i++) {
        buffer = new char[MAX_LINE_LENGTH];
        sprintf(buffer, "declare %s %s.%d", vec_type, vec_name, i);
        expanded_prog_lines->push_back(string(buffer));
    }

    return vec_size;    
}


int Preprocessor::expand_define_instruction(char *tokens[], int num_tokens, vector<string> *expanded_prog_lines) {

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


int Preprocessor::expand_vector_instruction(const OperationType& oper_type, const string& result, const string& operand1, const string& operand2,
    vector<string> *expanded_prog_lines) {

    int dimension = vector_dimensions->at(operand1);

    if (oper_type == OperationType::DOT)
        return expand_dot_product_instruction(result, operand1, operand2, dimension, expanded_prog_lines);

    else if (oper_type == OperationType::COMPONENT_WISE_ADD)
        return expand_component_wise_add_instruction(result, operand1, operand2, dimension, expanded_prog_lines);

    else if (oper_type == OperationType::COMPONENT_WISE_MUL)
        return expand_component_wise_mul_instruction(result, operand1, operand2, dimension, expanded_prog_lines);

    else if (oper_type == OperationType::SCALE_VECTOR) 
        return expand_scale_vector_instruction(result, operand1, operand2, dimension, expanded_prog_lines);

    else if (oper_type == OperationType::INCREMENT_VECTOR)
        return expand_increment_vector_instruction(result, operand1, operand2, dimension, expanded_prog_lines);

    else return INVALID_LINE;
}



/* ----------------------- Helper Macro Expansion Methods ------------------------ */




int Preprocessor::expand_unary_macro(const string& macro_name, const string& operand, const string& result, 
    vector<string> *expanded_prog_lines) {

    struct macro *macro = macros->at(macro_name);
    vector<string> *macro_lines = macro->lines;
    macro->num_references++;

    for (int i = 0; i < macro->num_lines; i++) {
        string dummy_line = macro_lines->at(i);
        string modified_line = substitute_dummy_names(dummy_line, macro->result, macro->operand1, "", result, operand, "", macro->num_references);
        expanded_prog_lines->push_back(modified_line);
    }

    return macro->num_lines;

}


int Preprocessor::expand_binary_macro(const string& macro_name, const string& operand1, const string& operand2, const string& result, 
    vector<string> *expanded_prog_lines) {

    struct macro *macro = macros->at(macro_name);
    vector<string> *macro_lines = macro->lines;
    macro->num_references++;

    for (int i = 0; i < macro->num_lines; i++) {
        string dummy_line = macro_lines->at(i);
        string modified_line = substitute_dummy_names(dummy_line, macro->result, macro->operand1, macro->operand2, result, operand1, operand2, macro->num_references);
        expanded_prog_lines->push_back(modified_line);
    }

    return macro->num_lines;

}


string Preprocessor::substitute_dummy_names(const string& dummy_line, string dummy_result, string dummy_op1, string dummy_op2,
    const string& result, const string& operand1, const string& operand2, int macro_num_references) {

    char line_copy[MAX_LINE_LENGTH];
    char *tokens[MAX_NUM_TOKENS];
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
        else if (!is_keyword(string(tokens[i])) && !is_constant(string(tokens[i]))) {
            modified_line.append(string(tokens[i]) + to_string(macro_num_references) + " ");
        }
        else {
            modified_line.append(string(tokens[i]) + " ");
        }
    }

    return modified_line;

}


bool Preprocessor::is_binary_macro(const string& name) {
    return is_valid_macro(name) && macros->at(name)->is_binary;
}


bool Preprocessor::is_unary_macro(const string& name) {
    return is_valid_macro(name) && macros->at(name)->is_binary == false;
}



/* ------------------------------------ Helper Vector Expansion Methods ----------------------------------- */



int Preprocessor::expand_dot_product_instruction(const string& result, const string& vector1, const string& vector2, int dimension,
    vector<string> *expanded_prog_lines) {

    const char *vec1 = vector1.c_str(), *vec2 = vector2.c_str(), *res = result.c_str();

    // declare and define intvars for all the component-wise multiplications
    char *buffer;
    for (int i = 0; i < dimension; i++) {
        buffer = new char[MAX_LINE_LENGTH];
        sprintf(buffer, "declare intvar %s.%d", res, i);
        expanded_prog_lines->push_back(string(buffer));
        sprintf(buffer, "define %s.%d = mul %s.%d %s.%d", res, i, vec1, i, vec2, i);
        expanded_prog_lines->push_back(string(buffer));
    }

    // if the operand vectors' dimension is 1, the dot product is equal to the single component-wise product
    if (dimension == 1) {
        buffer = new char[MAX_LINE_LENGTH];
        sprintf(buffer, "define %s = %s.0", res, res);
        expanded_prog_lines->push_back(string(buffer));

        return 3;
    }

    // accumulate the sum of all the component-wise products
    for (int j = 0; j < (dimension - 1); j++) {
        buffer = new char[MAX_LINE_LENGTH];
        sprintf(buffer, "declare intvar %s.%d", res, dimension + j);
        expanded_prog_lines->push_back(string(buffer));
        if (j == 0) {
            sprintf(buffer, "define %s.%d = add %s.%d %s.%d", res, dimension + j, res, 0, res, 1);
            expanded_prog_lines->push_back(string(buffer));
        } else {
            sprintf(buffer, "define %s.%d = add %s.%d %s.%d", res, dimension + j, res, dimension + j - 1, res, j + 1);
            expanded_prog_lines->push_back(string(buffer));
        }
            
    }

    // define the value of the final dot product
    buffer = new char[MAX_LINE_LENGTH];
    sprintf(buffer, "define %s = %s.%d", res, res, 2 * dimension - 2);
    expanded_prog_lines->push_back(string(buffer));
    return (4 * dimension - 1);

}


int Preprocessor::expand_component_wise_add_instruction(const string& result_vec, const string& vector1, const string& vector2, int dimension, 
    vector<string> *expanded_prog_lines) {

    const char *vec1 = vector1.c_str(), *vec2 = vector2.c_str(), *res = result_vec.c_str();

    // define the components of the result vector as the component-wise sums of the operand vectors
    char *buffer;
    for (int i = 0; i < dimension; i++) {
        buffer = new char[MAX_LINE_LENGTH];
        sprintf(buffer, "define %s.%d = add %s.%d %s.%d", res, i, vec1, i, vec2, i);
        expanded_prog_lines->push_back(string(buffer));
    }

    return dimension;

}


int Preprocessor::expand_component_wise_mul_instruction(const string& result_vec, const string& vector1, const string& vector2, int dimension, 
    vector<string> *expanded_prog_lines) {

    const char *vec1 = vector1.c_str(), *vec2 = vector2.c_str(), *res = result_vec.c_str();

    // define the components of the result vector as the component-wise products of the operand vectors
    char *buffer;
    for (int i = 0; i < dimension; i++) {
        buffer = new char[MAX_LINE_LENGTH];
        sprintf(buffer, "define %s.%d = mul %s.%d %s.%d", res, i, vec1, i, vec2, i);
        expanded_prog_lines->push_back(string(buffer));
    }

    return dimension;

}


int Preprocessor::expand_scale_vector_instruction(const string& result_vec, const string& vector1, const string& scaling_factor, int dimension, 
    vector<string> *expanded_prog_lines) {

    const char *vec = vector1.c_str(), *s_factor = scaling_factor.c_str(), *res = result_vec.c_str();

    // define the components of the result vector as the products of the operand's components and the scaling factor
    char *buffer;
    for (int i = 0; i < dimension; i++) {
        buffer = new char[MAX_LINE_LENGTH];
        sprintf(buffer, "define %s.%d = mul %s.%d %s", res, i, vec, i, s_factor);
        expanded_prog_lines->push_back(string(buffer));
    }

    return dimension;

}


int Preprocessor::expand_increment_vector_instruction(const string& result_vec, const string& vector1, const string& incrementing_factor, int dimension,
    vector<string> *expanded_prog_lines) {

    const char *vec = vector1.c_str(), *incr_factor = incrementing_factor.c_str(), *res = result_vec.c_str();

    // define the components of the result vector as the sums of the operand's components and the incrementing factor
    char *buffer;
    for (int i = 0; i < dimension; i++) {
        buffer = new char[MAX_LINE_LENGTH];
        sprintf(buffer, "define %s.%d = add %s.%d %s", res, i, vec, i, incr_factor);
        expanded_prog_lines->push_back(string(buffer));
    }

    return dimension;

}



/* -------------------------- Macro Parsing Methods --------------------------- */



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

    macro->num_references = 0;
    return 0;
    
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


int Preprocessor::parse_macro_subsequent_line(char line[], struct macro *macro) {
    
    if (line == NULL || macro == NULL) return OTHER_ERROR;

    // make a copy of the line
    char line_copy[MAX_LINE_LENGTH];
    strcpy(line_copy, line);
    // tokenize
    char *tokens[MAX_LINE_LENGTH];
    int num_tokens = tokenize_line(line_copy, tokens, delimiters);
    
    // temporarily declare or define this variable
    if (is_valid_declare_line(tokens, num_tokens) == 0) {
        variables->insert(make_pair(string(tokens[2]), get_variable_type(string(tokens[1]))));
        return 0;
    }

    if (is_valid_define_line(tokens, num_tokens) == 0) {
        defined_variables->insert(string(tokens[1]));
        return 0;
    }

    return INVALID_LINE;
}




/* -------------------------- Error Checking Methods -------------------------- */


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
            if (variables->count(second_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
            if (defined_variables->count(second_token) != 0) return VAR_DECLARED_TWICE;
        } else {
            if (vectors->count(second_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
            if (defined_variables->count(second_token) != 0) return VAR_DECLARED_TWICE;
        }

        // both the operands must have been declared and defined, and must be of the same dimension
        // for non-dot product, operations, the operands must also be of the same dimension as the result
        if (is_binary_vector_operation(fourth_token)) {
            
            if (num_tokens != 6) return INVALID_LINE;
            
            string fifth_token(tokens[4]), sixth_token(tokens[5]);

            if (vectors->count(fifth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
            if (vectors->count(sixth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
        
            if (vector_dimensions->at(fifth_token) != vector_dimensions->at(sixth_token)) return VECTORS_OF_DIFFERENT_DIMENSION;
            if (get_operation_type(fourth_token) != OperationType::DOT && vector_dimensions->at(fifth_token) != vector_dimensions->at(second_token))
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


bool Preprocessor::is_valid_macro(const string& name) {
    return macros->count(name) > 0;
}







void test_expand(char line[], Preprocessor *p) {
    vector<string> *expanded_prog_lines = new vector<string> ();
    int num_lines = p->expand_line(expanded_prog_lines, line);
    for (int i = 0; i < num_lines; i++) cout << expanded_prog_lines->at(i) << endl;
}


/* IF A MACRO IS CALLED TWICE IN A PROGRAM, LALA WILL BE DECLARED TWICE, IT WON'T WORK THE SECOND TIME */
void test_expand_macro() {
    char a[1000], b[1000], c[1000], d[1000], e[1000];
    strcpy(a, "#macro res = my_macro opone optwo; declare intvar lala; define lala = logistic opone; declare intvar q; define q = pow lala 3; define res = mul optwo q");
    strcpy(b, "declare input a");
    strcpy(c, "declare weight b");
    strcpy(d, "declare output c");
    strcpy(e, "define c = my_macro a b");
    Preprocessor *p = new Preprocessor();
    test_expand(a, p);
    test_expand(b, p);
    test_expand(c, p);
    test_expand(d, p);
    test_expand(e, p);
}

void test_expand_dot_product() {
    char a[1000], b[1000], c[1000], d[1000];
    strcpy(a, "declare_vector intvar z 3");
    strcpy(b, "declare_vector input a 3");
    strcpy(c, "declare output b");
    strcpy(d, "define b = dot z a");
    Preprocessor *p = new Preprocessor();
    test_expand(a, p);
    test_expand(b, p);
    test_expand(c, p);
    test_expand(d, p);
}

void test_expand_primitive_define() {
    char a[1000], b[1000], c[1000], d[1000];
    strcpy(a, "declare intvar z");
    strcpy(b, "declare input a");
    strcpy(c, "declare weight b");
    strcpy(d, "define z = logistic b");
    Preprocessor *p = new Preprocessor();
    test_expand(a, p);
    test_expand(b, p);
    test_expand(c, p);
    test_expand(d, p);
}


void test_expand_declare() {
    char a[1000], b[1000];
    strcpy(a, "declare exp_output eo");
    strcpy(b, "declare_vector weight w 5");
    Preprocessor *p = new Preprocessor ();
    test_expand(a, p);
    test_expand(b, p);
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
    //test_macro_parsing_helper(a);

    strcpy(b, "#macro res = my_macro a b; declare intvar z; define z = 3; define a = add b 1");
    test_macro_parsing_helper(b);

    strcpy(c, "#macro res = my_macro a b; res = mul a b");
    //test_macro_parsing_helper(c);

    strcpy(d, "define res = my_macro a b; declare intvar q; define res = add q p;");
    //test_macro_parsing_helper(d);
}


int main(int argc, char *argv[]) {
    //test_macro_parsing();
    test_expand_macro();
    return 0;
}




