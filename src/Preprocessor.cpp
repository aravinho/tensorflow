#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

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


Preprocessor::~Preprocessor() {
    delete variables;
    delete vectors;
    delete vector_dimensions;
    delete defined_variables;
    for (unordered_map<string, struct macro*>::iterator it = macros->begin();
        it != macros->end(); ++it) {

        delete it->second;
    }
    delete macros;
}


/* ------------------------------- Main Methods ---------------------------- */

int Preprocessor::expand_program(const string& prog_filename, const string& expanded_prog_filename) {

    if (!is_valid_file_name(prog_filename)) {
        cerr << "\nInvalid Program file name: " << prog_filename << endl << endl;
        return INVALID_FILE_NAME;
    }

    ifstream prog(prog_filename);
    ofstream exp_prog(expanded_prog_filename);

    // buffer into which we read a line from the file
    string prog_line;

    // indicates how many lines were generated from a given line in the Shape Program.
    // 1 if no expansion was necessary, 0 for an empty line, -1 if the Shape Program line was invalid.
    int num_lines_expanded;

    // expand each line
    int line_num = 0;
    while(!prog.eof()) {

        getline(prog, prog_line);
        num_lines_expanded = expand_line(prog_line, exp_prog);
        // if there is an error expanding this line:
        // print the error message, clear the Expanded Program file, and exit
        if (num_lines_expanded < 0) {
            cerr << "\nERROR, Line " << line_num << ":" << endl;
            cerr << prog_line << endl;
            cerr << get_error_message(num_lines_expanded) << endl << endl;
            prog.close();
            exp_prog.close();
            ofstream clear_exp_prog(expanded_prog_filename);
            clear_exp_prog.close();
            return num_lines_expanded;
        }

        line_num++;

    }

    prog.close();
    exp_prog.close();

    return 0;

}


int Preprocessor::expand_line(const string& prog_line, ofstream& exp_prog) {
    
    // edge error cases
    if (!exp_prog.is_open()) return OTHER_ERROR;
    if (prog_line.compare("") == 0) return 0;

    // tokenize the line
    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(prog_line, tokens, " ");
    if (num_tokens < 3) return INVALID_LINE;

    // grab the instruction type, and act accordingly
    if (!is_valid_instruction(tokens->at(0))) return INVALID_LINE;
    InstructionType inst_type = get_instruction_type(tokens->at(0));


    // macro instructions need to be parsed and stored, but are not written to the expanded program
    // declare instructions need no expanding
    // define instructions need expanding if they involve macros or vector operations
    // declare_vector and define_vector instructions always need expanding into their components

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
    
    // as soon as we see a non-macro-definition, there can be no more macros
    macros_done = true;

    if (inst_type == InstructionType::DECLARE) {

        // verify the line is a valid DECLARE instruction
        int valid_declare_line = is_valid_declare_line(prog_line);
        if (valid_declare_line < 0) return valid_declare_line;

        // copy the line directly. It needs no expanding
        exp_prog << prog_line << endl;

        // record the type of this variable
        // if it's an input, weight or exp_output, mark it as defined
        VariableType var_type = get_variable_type(tokens->at(1));
        string var_name = tokens->at(2);
        variables->insert(make_pair(var_name, var_type));
        if (var_type == VariableType::INPUT || var_type == VariableType::WEIGHT || var_type == VariableType::EXP_OUTPUT)
            defined_variables->insert(var_name);

        return 1;
    }
    
    if (inst_type == InstructionType::DEFINE) {

        // verify the line is a valid DEFINE instruction
        int valid_define_line = is_valid_define_line(prog_line);
        if (valid_define_line < 0) return valid_define_line;

        // expand the line
        int num_lines = expand_define_instruction(prog_line, exp_prog);
        if (num_lines < 0) return num_lines;

        // mark this variable as defined
        string var_name = tokens->at(1);
        defined_variables->insert(var_name);
        return num_lines;

    }

    if (inst_type == InstructionType::DEFINE_VECTOR) {

        // verify the line is a valid DEFINE instruction
        int valid_define_vector_line = is_valid_define_vector_line(prog_line);
        if (valid_define_vector_line < 0) return valid_define_vector_line;

        // expand the line
        int num_lines = expand_define_vector_instruction(prog_line, exp_prog);
        if (num_lines < 0) return num_lines;

        // mark this vector as defined
        string var_name = tokens->at(1);
        defined_variables->insert(var_name);
        return num_lines;
    }
    
    if (inst_type == InstructionType::DECLARE_VECTOR) {

        // verify the line is a valid DECLARE_VECTOR instruction
        int valid_declare_vector_line = is_valid_declare_vector_line(prog_line);
        if (valid_declare_vector_line < 0) return valid_declare_vector_line;

        // write the expanded component declarations into expanded_shape_lines
        // grab the size of the vector from the return value of expand_declare_vector_instruction
        int vector_size = expand_declare_vector_instruction(prog_line, exp_prog);

        // record the type and dimension of this vector
        string vec_name = tokens->at(2);
        VariableType vec_type = get_variable_type(tokens->at(1));
        int vec_size = stoi(tokens->at(3));
        vectors->insert(make_pair(vec_name, vec_type));
        vector_dimensions->insert(make_pair(vec_name, vec_size));

        if (vec_type == VariableType::INPUT || vec_type == VariableType::WEIGHT || vec_type == VariableType::EXP_OUTPUT) {
            defined_variables->insert(vec_name);
        }

        // the number of expanded lines is precisely the dimension of the vector (one line per component)
        return vector_size;

    }

    return INVALID_LINE;

}


/* ------------------------- Main Expansion Methods ------------------------- */


int Preprocessor::expand_declare_vector_instruction(const string& line, ofstream &exp_prog) {

    if (line.compare("") == 0) return 0;

    // tokenize the line
    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(line, tokens, " ");
    if (num_tokens != 4) return INVALID_LINE;

    // grab the vector name, type and size
    string vec_type = tokens->at(1), vec_name = tokens->at(2);
    int vec_size = stoi(tokens->at(3));
    VariableType vec_var_type = get_variable_type(vec_type);
    
    // expand into declarations of components
    // place all the components into the Variables map (marking them as declared)
    // if this vector is input, weight or exp_output, mark all the components as defined
    string component_name;
    for (int i = 0; i < vec_size; i++) {
        component_name = get_vector_component_name(vec_name, i);
        exp_prog << "declare " << vec_type << " " << component_name << endl;
        variables->insert(make_pair(component_name, vec_var_type));
        if (vec_var_type == VariableType::INPUT || vec_var_type == VariableType::WEIGHT || vec_var_type == VariableType::EXP_OUTPUT) {
            defined_variables->insert(component_name);
        }
    }

    return vec_size;    
}


int Preprocessor::expand_define_instruction(const string& line, ofstream &exp_prog) {

    if (line.compare("") == 0) return 0;

    // tokenize the line
    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(line, tokens, " ");
    if (num_tokens < 4 || num_tokens > 6) return INVALID_LINE;

    string var_name = tokens->at(1), operation = tokens->at(3);

    if (is_dot_product(operation)) {
        string operand1 = tokens->at(4);
        string operand2 = tokens->at(5);
        int dimension = vector_dimensions->at(operand1);
        return expand_dot_product_instruction(var_name, operand1, operand2, dimension, exp_prog);
    }

    if (is_reduce_vector(operation)) {
        string vec_operand = tokens->at(4);
        string func_operand = tokens->at(5);
        int dimension = vector_dimensions->at(vec_operand);
        return expand_reduce_vector_instruction(var_name, vec_operand, func_operand, dimension, exp_prog);
    }

    else if (is_valid_macro(operation)) {
        if (num_tokens == 5) {
            string operand = tokens->at(4);
            return expand_unary_macro(operation, operand, var_name, exp_prog);
        }
        if (num_tokens == 6) {
            string operand1 = tokens->at(4);
            string operand2 = tokens->at(5);
            return expand_binary_macro(operation, operand1, operand2, var_name, exp_prog);
        }

    }

    else if (is_constant(operation) || is_valid_var_name(operation) || is_valid_primitive(operation)) {
        exp_prog << line << endl;
        return 1;
    }

    return INVALID_LINE;

}

int Preprocessor::expand_define_vector_instruction(const string& line, ofstream &exp_prog) {

    if (line.compare("") == 0) return 0;

    // tokenize the line
    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(line, tokens, " ");
    if (num_tokens < 5 || num_tokens > 6) return INVALID_LINE;

    string var_name = tokens->at(1), operation = tokens->at(3);
    string operand1 = tokens->at(4);
    string operand2 = "";

    // determine whether the operation is a primitive or a macro
    bool operation_is_primitive = is_valid_primitive(operation);
    bool operation_is_macro = is_valid_macro(operation);
    if (!operation_is_primitive && !operation_is_macro) return INVALID_LINE;

    // determine whether the operation requires 1 vector, a vector and a scalar, or 2 vectors
    bool unary_vector_operation = true;
    bool vector_scalar_operation = false;
    bool binary_vector_operation = false;

    if (num_tokens == 6) {
        unary_vector_operation = false;
        operand2 = tokens->at(5);

        if (vector_dimensions->count(operand2) > 0) {
            binary_vector_operation = true;
        } else {
            vector_scalar_operation = true;
        }
    }

    // determine the dimensions we are working with
    int dimension = vector_dimensions->at(operand1);

    // expand into component instructions
    for (int i = 0; i < dimension; i++) {

        string operand1_component = get_vector_component_name(operand1, i);
        string operand2_component = get_vector_component_name(operand2, i);
        string result_component = get_vector_component_name(var_name, i);

        if (unary_vector_operation) {
            if (operation_is_primitive) {
                exp_prog << "define " << result_component << " = " << operation << " " << operand1_component << endl;
            } else if (operation_is_macro) {
                expand_unary_macro(operation, operand1_component, result_component, exp_prog);
            }
        }

        else if (vector_scalar_operation) {
            if (operation_is_primitive) {
                exp_prog << "define " << result_component << " = " << operation << " " <<
                operand1_component << " " << operand2 << endl;
            } else if (operation_is_macro) {
                expand_binary_macro(operation, operand1_component, operand2, result_component, exp_prog);
            }
        }

        else if (binary_vector_operation) {
            if (operation_is_primitive) {
                exp_prog << "define " << result_component << " = " << operation << " " <<
                operand1_component << " " << operand2_component << endl;
            } else if (operation_is_macro) {
                expand_binary_macro(operation, operand1_component, operand2_component, result_component, exp_prog);
            }
        }

        // mark this component as defined
        defined_variables->insert(result_component);
    }

    return dimension;

}



int Preprocessor::expand_vector_operation(const OperationType& oper_type, const string& result, const string& operand1, const string& operand2,
    ofstream& exp_prog) {

    int dimension = vector_dimensions->at(operand1);

    if (oper_type == OperationType::DOT)
        return expand_dot_product_instruction(result, operand1, operand2, dimension, exp_prog);

    else if (oper_type == OperationType::COMPONENT_WISE_ADD)
        return expand_component_wise_add_instruction(result, operand1, operand2, dimension, exp_prog);

    else if (oper_type == OperationType::COMPONENT_WISE_MUL)
        return expand_component_wise_mul_instruction(result, operand1, operand2, dimension, exp_prog);

    else if (oper_type == OperationType::SCALE_VECTOR) 
        return expand_scale_vector_instruction(result, operand1, operand2, dimension, exp_prog);

    else if (oper_type == OperationType::INCREMENT_VECTOR)
        return expand_increment_vector_instruction(result, operand1, operand2, dimension, exp_prog);

    else return INVALID_LINE;
}



/* ----------------------- Helper Macro Expansion Methods ------------------------ */




int Preprocessor::expand_unary_macro(const string& macro_name, const string& operand, const string& result, 
    ofstream& exp_prog) {

    struct macro *macro = macros->at(macro_name);
    vector<string> *macro_lines = macro->lines;
    

    for (int i = 0; i < macro->num_lines; i++) {
        string dummy_line = macro_lines->at(i);
        string modified_line = substitute_dummy_names(dummy_line, macro->result, macro->operand1, "", result, operand, "", macro->num_references);
        exp_prog << modified_line << endl;
    }

    macro->num_references++;
    return macro->num_lines;

}


int Preprocessor::expand_binary_macro(const string& macro_name, const string& operand1, const string& operand2, const string& result, 
    ofstream& exp_prog) {

    struct macro *macro = macros->at(macro_name);
    vector<string> *macro_lines = macro->lines;
    
    for (int i = 0; i < macro->num_lines; i++) {
        string dummy_line = macro_lines->at(i);
        string modified_line = substitute_dummy_names(dummy_line, macro->result, macro->operand1, macro->operand2, result, operand1, operand2, macro->num_references);
        exp_prog << modified_line << endl;
    }

    macro->num_references++;
    return macro->num_lines;

}


string Preprocessor::substitute_dummy_names(const string& dummy_line, string dummy_result, string dummy_op1, string dummy_op2,
    const string& result, const string& operand1, const string& operand2, int macro_num_references) {

    // tokenize the line
    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(dummy_line, tokens, " ");
    if (num_tokens < 3) return "";

    string modified_line = "";

    for (int i = 0; i < num_tokens; i++) {
        string token = tokens->at(i);

        if (token.compare(dummy_result) == 0) {
            modified_line.append(result);
        }
        else if (token.compare(dummy_op1) == 0) {
            modified_line.append(operand1);
        }
        else if (token.compare(dummy_op2) == 0) {
            modified_line.append(operand2);
        }

        // if the token is one of the intvars used in the macro
        // give it a unique name
        // if the macro defines c, and this token is an intvar p, and this is the 0th reference to this macro:
        // the line becomes "declare intvar c_p0"
        else if (!is_keyword(token) && !is_constant(token)) {
            modified_line.append(result + "_" + token + to_string(macro_num_references));
        }
        else {
            modified_line.append(token);
        }

        if (i < num_tokens - 1) {
            modified_line.append(" ");
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
    ofstream& exp_prog) {

    // declare and define intvars for all the component-wise multiplications
    for (int i = 0; i < dimension; i++) {
        exp_prog << "declare intvar " << result << "." << i << endl;
        exp_prog << "define " << result << "." << i << " = mul " << vector1 << "." << i << " " << vector2 << "." << i << endl;
    }

    // if the operand vectors' dimension is 1, the dot product is equal to the single component-wise product
    if (dimension == 1) {
        exp_prog << "define " << result << " = " << result << ".0" << endl;
        return 3;
    }

    // accumulate the sum of all the component-wise products
    for (int j = 0; j < (dimension - 1); j++) {
        exp_prog << "declare intvar " << result << "." << dimension + j << endl;
        if (j == 0) {
            exp_prog << "define " << result << "." << dimension + j << " = add " << result << "." << 0 << " " << result << "." << 1 << endl;
        } else {
            exp_prog << "define " << result << "." << dimension + j << " = add " << result << "." << dimension + j - 1 << " " << result << "." << j + 1 << endl;
        }
            
    }

    // define the value of the final dot product
    exp_prog << "define " << result << " = " << result << "." << 2 * dimension - 2 << endl;
    return (4 * dimension - 1);

}


int Preprocessor::expand_reduce_vector_instruction(const string& result, const string& vec,
    const string& func, int dimension, ofstream& exp_prog) {

    // determine whether the operation is a primitive or a macro
    bool func_is_primitive = is_valid_primitive(func);
    bool func_is_macro = is_valid_macro(func);
    if (!func_is_primitive && !func_is_macro) return INVALID_LINE;

    if (dimension == 0) return 0;
    // if the vector has one component, RESULT simply equals that component
    if (dimension == 1) {
        string component_zero = get_vector_component_name(vec, 0);
        exp_prog << "define " << result << " = " << component_zero << endl;
        return 1;
    }

    // this counter will track the number of lines required to expand this vector reduction
    int num_lines = 0;

    // iterate from the 1st to the final component of the vector
    // declare an intvar NEW_ACCUMULATION
    // define this intvar as the sum of the i-th component and the running accumulation (PREV_ACCUMULATION)
    string ith_component, new_accumulation, prev_accumulation;
    for (int i = 1; i < dimension; i++) {

        // grab the names for the ith component, new accumulation and prev accumulation
        // if this is the first iteration, the previous accumulation is just the zeroth component
        ith_component = get_vector_component_name(vec, i);
        new_accumulation = get_intermediate_name(result, i - 1);
        if (i == 1) {
            prev_accumulation = get_vector_component_name(vec, 0);
        } else {
            prev_accumulation = get_intermediate_name(result, i - 2);
        }

        // declare the intvar for NEW_ACCUMULATION
        exp_prog << "declare intvar " << new_accumulation << endl;
        num_lines++;

        // define NEW_ACCUMULATION
        // if FUNC is a macro, this definition requires macro expansion
        if (func_is_primitive) {
            exp_prog << "define " << new_accumulation << " = " << func << " " << prev_accumulation << " " << ith_component << endl;
            num_lines++;
        }
        else if (func_is_macro) {
            num_lines += expand_binary_macro(func, prev_accumulation, ith_component, new_accumulation, exp_prog);
        }

    }

    // define RESULT as the most recent accumulation intvar
    exp_prog << "define " << result << " = " << new_accumulation << endl;
    num_lines++;
 
    return num_lines;
}


int Preprocessor::expand_component_wise_add_instruction(const string& result_vec, const string& vector1, const string& vector2, int dimension, 
    ofstream& exp_prog) {

    // define the components of the result vector as the component-wise sums of the operand vectors
    for (int i = 0; i < dimension; i++) {
        exp_prog << "define " << result_vec << "." << i << " = add " << vector1 << "." << i << " " << vector2 << "." << i << endl;
    }

    return dimension;

}


int Preprocessor::expand_component_wise_mul_instruction(const string& result_vec, const string& vector1, const string& vector2, int dimension, 
    ofstream& exp_prog) {

    // define the components of the result vector as the component-wise products of the operand vectors
    for (int i = 0; i < dimension; i++) {
        exp_prog << "define " << result_vec << "." << i << " = mul " << vector1 << "." << i << " " << vector2 << "." << i << endl;
    }

    return dimension;

}


int Preprocessor::expand_scale_vector_instruction(const string& result_vec, const string& vector1, const string& scaling_factor, int dimension, 
    ofstream& exp_prog) {

    // define the components of the result vector as the products of the operand's components and the scaling factor
    for (int i = 0; i < dimension; i++) {
        exp_prog << "define " << result_vec << "." << i << " = mul " << vector1 << "." << i << " " << scaling_factor << endl;
    }

    return dimension;

}


int Preprocessor::expand_increment_vector_instruction(const string& result_vec, const string& vector1, const string& incrementing_factor, int dimension,
    ofstream& exp_prog) {

    // define the components of the result vector as the sums of the operand's components and the incrementing factor
    for (int i = 0; i < dimension; i++) {
        exp_prog << "define " << result_vec << "." << i << " = add " << vector1 << "." << i << " " << incrementing_factor << endl;
    }

    return dimension;

}



/* -------------------------- Macro Parsing Methods --------------------------- */



int Preprocessor::parse_macro_line(const string& macro_line, struct macro *macro) {
    
    if (macro == NULL) return OTHER_ERROR;
    if (macro_line.compare("") == 0) return 0;

    // tokenize the large macro line into its sublines (separated by semi-colons)
    vector<string> *sub_macro_lines = new vector<string>();
    int num_sub_lines = tokenize_line(macro_line, sub_macro_lines, ";");
    if (num_sub_lines < 2) return INVALID_LINE;
    
    // make sure the first sub line is valid and parse it
    int first_line_valid = parse_macro_first_line(sub_macro_lines->at(0), macro);
    if (first_line_valid < 0) return first_line_valid;

    macro->lines = new vector<string>();
    macro->num_lines = 0;

    // validate and parse subsequent sub lines.
    int subsequent_line_valid;
    for (int i = 1; i < num_sub_lines; i++) {
        subsequent_line_valid = parse_macro_subsequent_line(sub_macro_lines->at(i), macro);
        if (subsequent_line_valid < 0) return subsequent_line_valid;
        macro->lines->push_back(string(sub_macro_lines->at(i)));
        macro->num_lines++;
    }

    macro->num_references = 0;
    return 0;
    
}


int Preprocessor::parse_macro_first_line(const string& first_line, struct macro *macro) {

    if (macro == NULL) return OTHER_ERROR;
    if (first_line.compare("") == 0) return 0;

    // tokenize
    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(first_line, tokens, " ");

    // check for trivial errors
    if (num_tokens < 5 || num_tokens > 6) return INVALID_LINE;

    // grab the tokens in strings
    string first_token = tokens->at(0), second_token = tokens->at(1), third_token = tokens->at(2);
    string fourth_token = tokens->at(3), fifth_token = tokens->at(4);
    string sixth_token = "";

    // make sure the line format is correct
    if (first_token.compare("#macro") != 0) return INVALID_LINE;
    if (!is_valid_var_name(second_token)) return INVALID_VAR_NAME;
    if (third_token.compare("=") != 0) return INVALID_LINE;
    if (!is_valid_macro_name(fourth_token)) return INVALID_MACRO_NAME;
    if (!is_valid_var_name(fifth_token) || second_token.compare(fifth_token) == 0) return INVALID_LINE;
    if (num_tokens == 6) {
        sixth_token = tokens->at(5);
        if (!is_valid_var_name(sixth_token) || second_token.compare(sixth_token) == 0) return INVALID_LINE;
    }

    // populate the fields of the macro struct
    macro->name = fourth_token;
    macro->is_binary = (num_tokens == 6 ? true : false);
    macro->result = second_token;
    macro->operand1 = fifth_token;
    macro->operand2 = sixth_token;

    // declare the result and operand variables temporarily
    // the operand variables cannot be defined in the macro
    variables->insert(make_pair(macro->result, VariableType::INTVAR));
    variables->insert(make_pair(macro->operand1, VariableType::INTVAR));
    if (macro->is_binary) variables->insert(make_pair(macro->operand2, VariableType::INTVAR));

    defined_variables->insert(macro->operand1);
    if (macro->is_binary) defined_variables->insert(macro->operand2);


    return 0;

}


int Preprocessor::parse_macro_subsequent_line(const string& line, struct macro *macro) {
    
    if (macro == NULL) return OTHER_ERROR;
    if (line.compare("") == 0) return 0;

    // tokenize
    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(line, tokens, " ");
    if (num_tokens < 3 || num_tokens > 6) return INVALID_LINE;

    // temporarily declare or define this variable
    if (is_valid_declare_line(line) == 0) {
        string var_name = tokens->at(2);
        VariableType var_type = get_variable_type(tokens->at(1));
        if (var_type != VariableType::INTVAR) return BAD_VAR_TYPE;
        variables->insert(make_pair(var_name, var_type));
        return 0;
    }

    if (is_valid_define_line(line) == 0) {
        string var_name = tokens->at(1);
        defined_variables->insert(var_name);
        return 0;
    }

    return INVALID_LINE;
}




/* -------------------------- Error Checking Methods -------------------------- */


int Preprocessor::is_valid_declare_line(const string& line) {

    if (line.compare("") == 0) return OTHER_ERROR;

    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(line, tokens, " ");
    if (num_tokens != 3) return INVALID_LINE;

    if (tokens->at(0).compare("declare") != 0) return INVALID_LINE;
    if (get_variable_type(tokens->at(1)) == VariableType::INVALID_VAR_TYPE) return BAD_VAR_TYPE;
    if (!is_valid_var_name(tokens->at(2))) return INVALID_VAR_NAME;
    if (variables->count(tokens->at(2)) != 0) return VAR_DECLARED_TWICE;

    return 0;
}


int Preprocessor::is_valid_declare_vector_line(const string& line) {

    if (line.compare("") == 0) return OTHER_ERROR;

    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(line, tokens, " ");
    if (num_tokens != 4) return INVALID_LINE;

    if (tokens->at(0).compare("declare_vector") != 0) return INVALID_LINE;
    if (get_variable_type(tokens->at(1)) == VariableType::INVALID_VAR_TYPE) return BAD_VAR_TYPE;
    if (!is_valid_var_name(tokens->at(2))) return INVALID_VAR_NAME;
    if (vectors->count(tokens->at(2)) != 0 || variables->count(tokens->at(2)) != 0) return VAR_DECLARED_TWICE;
    if (!is_int(tokens->at(3)) || !is_valid_vector_size(stoi(tokens->at(3)))) return BAD_VECTOR_SIZE;

    return 0;

}


int Preprocessor::is_valid_define_line(const string& line) {

    if (line.compare("") == 0) return OTHER_ERROR;

    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(line, tokens, " ");
    if (num_tokens < 4 || num_tokens > 6) return INVALID_LINE;

    string first_token = tokens->at(0);
    string second_token = tokens->at(1);
    string third_token = tokens->at(2);

    // every define instruction resembles "define <var_name> = ..."
    if (first_token.compare("define") != 0 || third_token.compare("=") != 0) {
        return INVALID_LINE;
    }

    // the variable being defined must be a valid variable name
    // OR it must be the component of an already declared vector
    if (!is_valid_var_name(second_token) && !is_vector_component(second_token)) {
        return INVALID_VAR_NAME;
    }


    // input, weight and expected output variables cannot be defined
    VariableType var_type;
    if (variables->count(second_token) != 0) {
        var_type = variables->at(second_token);
    } 
    else {
        return VAR_DEFINED_BEFORE_DECLARED;
    }
    if (var_type == VariableType::INPUT || var_type == VariableType::WEIGHT || var_type == VariableType::EXP_OUTPUT)
        return CANNOT_DEFINE_I_W_EO;

    string fourth_token = tokens->at(3);


    // a variable could be defined as a constant
    if (is_constant(fourth_token)) {
        // the variable being defined must have been declared, but cannot have been defined
        if (variables->count(second_token) == 0) return VAR_DEFINED_BEFORE_DECLARED;
        if (defined_variables->count(second_token) != 0) return VAR_DEFINED_TWICE;
        return (num_tokens == 4 ? 0 : INVALID_LINE);
    }

    // a variable could be defined as an operation of 1 or 2 constants/variables
    // this operation might be a primitive or might be a user defined macro
    if (is_valid_primitive(fourth_token) || is_valid_macro(fourth_token)) {

        // the variable being defined must have been declared, but cannot have been defined
        if (variables->count(second_token) == 0) return VAR_DEFINED_BEFORE_DECLARED;
        if (defined_variables->count(second_token) != 0) return VAR_DEFINED_TWICE;
        
        if (is_binary_primitive(fourth_token) || is_binary_macro(fourth_token)) {
            if (num_tokens != 6) return INVALID_LINE;

            string fifth_token = tokens->at(4), sixth_token = tokens->at(5);
            bool first_operand_constant = is_constant(fifth_token);
            bool second_operand_constant = is_constant(sixth_token);

            if (!first_operand_constant)
                if (defined_variables->count(fifth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;

            if (!second_operand_constant)
                if (defined_variables->count(sixth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;

            return 0;

        }

        else if (is_unary_primitive(fourth_token) || is_unary_macro(fourth_token)) {
            
            if (num_tokens != 5) return INVALID_LINE;

            string fifth_token = tokens->at(4);
            bool first_operand_constant = is_constant(fifth_token);

            if (!first_operand_constant)
                if (defined_variables->count(fifth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;

            return 0;

        }

    }

    // a variable could be defined as the dot product of two previously defined vectors
    if (is_dot_product(fourth_token)) {

        // the variable being defined must have been declared, but cannot have been defined
        // the result variable must be a scalar, not a vector
        if (variables->count(second_token) == 0) return VAR_DEFINED_BEFORE_DECLARED;
        if (defined_variables->count(second_token) != 0) return VAR_DEFINED_TWICE;
        if (vectors->count(second_token) != 0) return INVALID_LINE;

        if (num_tokens != 6) return INVALID_LINE;
        string fifth_token = tokens->at(4), sixth_token = tokens->at(5);

        // both the operand vectors must have been defined (or have all its components defined)
        if (vectors->count(fifth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
        if (defined_variables->count(fifth_token) == 0 && !all_components_defined(fifth_token)) return VAR_REFERENCED_BEFORE_DEFINED;
        if (vectors->count(sixth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
        if (defined_variables->count(sixth_token) == 0 && !all_components_defined(sixth_token)) return VAR_REFERENCED_BEFORE_DEFINED;

        // both the operand vectors must be of the same dimension
        if (vector_dimensions->at(fifth_token) != vector_dimensions->at(sixth_token)) return VECTORS_OF_DIFFERENT_DIMENSION;


        // if the first operand is a vector with all its operands defined, but the vector itself isn't defined,
        // mark the vector as defined
        // This ensures that a vector defined component-wise is not marked as defined 
        //  until it is used as an operand for another vector
        if (defined_variables->count(fifth_token) == 0 && all_components_defined(fifth_token)) {
            defined_variables->insert(fifth_token);
        }

        // if the second operand is a vector with all its operands defined, but the vector itself isn't defined,
        // mark the vector as defined
        // This ensures that a vector defined component-wise is not marked as defined 
        //  until it is used as an operand for another vector
        if (defined_variables->count(sixth_token) == 0 && all_components_defined(sixth_token)) {
            defined_variables->insert(sixth_token);
        }

        return 0;

    }


    if (is_reduce_vector(fourth_token)) {

        if (num_tokens != 6) return INVALID_LINE;
        string fifth_token = tokens->at(4), sixth_token = tokens->at(5);

        // the variable being defined must have been declared, but cannot have been defined
        // the result variable must be a scalar, not a vector
        if (variables->count(second_token) == 0) return VAR_DEFINED_BEFORE_DECLARED;
        if (defined_variables->count(second_token) != 0) return VAR_DEFINED_TWICE;
        if (vectors->count(second_token) != 0) return INVALID_LINE;

        // the operand vector must have been defined (or have all its components defined)
        if (vectors->count(fifth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
        if (defined_variables->count(fifth_token) == 0 && !all_components_defined(fifth_token)) return VAR_REFERENCED_BEFORE_DEFINED;

        // the 6th token must be a valid binary primitive or a valid binary macro
        if (!is_binary_primitive(sixth_token) && !is_binary_macro(sixth_token)) return INVALID_LINE;

        // if the operand vector is a vector with all its operands defined, but the vector itself isn't defined,
        // mark the vector as defined
        // This ensures that a vector defined component-wise is not marked as defined 
        //  until it is used as an operand for another vector
        if (defined_variables->count(fifth_token) == 0 && all_components_defined(fifth_token)) {
            defined_variables->insert(fifth_token);
        }

        return 0;

    }


    // a variable could be defined as equivalent to another variable
    // this other variable must have been declared already
    if (is_valid_var_name(fourth_token)) {

        // the variable being defined must have been declared, but cannot have been defined
        if (variables->count(second_token) == 0) return VAR_DEFINED_BEFORE_DECLARED;
        if (defined_variables->count(second_token) != 0) return VAR_DEFINED_TWICE;

        if (num_tokens != 4) return INVALID_LINE;
        if (defined_variables->count(fourth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
        return 0;
    }


    return INVALID_LINE;

}


int Preprocessor::is_valid_define_vector_line(const string& line) {

    if (line.compare("") == 0) return OTHER_ERROR;

    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(line, tokens, " ");
    if (num_tokens < 5 || num_tokens > 6) return INVALID_LINE;

    string first_token = tokens->at(0);
    string second_token = tokens->at(1);
    string third_token = tokens->at(2);

    // every define instruction resembles "define <vec_name> = ..."
    if (first_token.compare("define_vector") != 0 || third_token.compare("=") != 0) {
        return INVALID_LINE;
    }
    // the name of the vector being defined must be valid
    if (!is_valid_var_name(second_token)) {
        return INVALID_VAR_NAME;
    }


    // the vector being defined must have been previously declared
    // grab its type
    VariableType vec_type;
    if (vectors->count(second_token) != 0) {
        vec_type = vectors->at(second_token);
    } else {
        return VAR_DEFINED_BEFORE_DECLARED;
    }

    // input, weight and expected output vectors cannot be defined
    if (vec_type == VariableType::INPUT || vec_type == VariableType::WEIGHT || vec_type == VariableType::EXP_OUTPUT)
        return CANNOT_DEFINE_I_W_EO;

    // vectors that have previously been defined cannot be redefined
    // none of the components can have been defined previously
    if (defined_variables->count(second_token) != 0) return VAR_DEFINED_TWICE;
    if (has_defined_components(second_token)) return VAR_DEFINED_TWICE;


    string fourth_token = tokens->at(3);

    // a vector could be defined as a binary primitive/macro operation on:
    // two vectors ("define_vector z = add x y", where z, x and y are vectors)
    // a vector and a scalar variable ("define_vector z = my_macro x q", where z and x are vectors, q is a scalar, and my_macro is a binary macro)
    // a vector and a constant ("define vector_z = pow x 3")
    if (is_binary_primitive(fourth_token) || is_binary_macro(fourth_token)) {

        if (num_tokens != 6) return INVALID_LINE;

        string fifth_token = tokens->at(4);
        string sixth_token = tokens->at(5);

        // the first operand must be a vector
        // it must be declared
        // the vector must have defined, or else all its components must have been defined
        // this operand vector must be of the same dimension as the result vector
        if (vectors->count(fifth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
        if (defined_variables->count(fifth_token) == 0 && !all_components_defined(fifth_token)) return VAR_REFERENCED_BEFORE_DEFINED; 
        if (vector_dimensions->at(fifth_token) != vector_dimensions->at(second_token)) return VECTORS_OF_DIFFERENT_DIMENSION;

        // if the first operand is a vector with all its operands defined, but the vector itself isn't defined,
        // mark the vector as defined
        // This ensures that a vector defined component-wise is not marked as defined 
        //  until it is used as an operand for another vector
        if (defined_variables->count(fifth_token) == 0 && all_components_defined(fifth_token)) {
            defined_variables->insert(fifth_token);
        }


        // the second operand must either be a defined vector, a defined scalar variable, or a constant

        // if second operand is a vector, it must have been declared
        // it must have been defined, or else all its components must have been defined
        // it must be of the same dimension as the result and first operand vectors
        if (vectors->count(sixth_token) != 0) {
            if (defined_variables->count(sixth_token) == 0 && !all_components_defined(sixth_token)) return VAR_REFERENCED_BEFORE_DEFINED;
            if (vector_dimensions->at(fifth_token) != vector_dimensions->at(sixth_token)) return VECTORS_OF_DIFFERENT_DIMENSION;

            // if the second operand is a vector with all its operands defined, but the vector itself isn't defined,
            // mark the vector as defined
            // This ensures that a vector defined component-wise is not marked as defined 
            //  until it is used as an operand for another vector
            if (defined_variables->count(sixth_token) == 0 && all_components_defined(sixth_token)) {
                defined_variables->insert(sixth_token);
            }
            return 0;
        }

        // if the second operand is a constant, return 0
        else if (is_constant(sixth_token)) {
            return 0;
        }

        // if the second operand is not a vector or a constant,
        // make sure it is a variable, and has been defined
        else if (is_valid_var_name(sixth_token)) {
            if (variables->count(sixth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
            if (defined_variables->count(sixth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED;
            return 0;
        }

        else {
            return INVALID_LINE;
        }
    }


    // a vector could be defined as a unary primitive/macro operation on one vector:
    // ("define_vector z = logistic x", where z and x are vectors)
    // ("define_vector z = my_unary_macro x", where z and x are vectors, and my_unary_macro is a unary macro)
    if (is_unary_primitive(fourth_token) || is_unary_macro(fourth_token)) {

        if (num_tokens != 5) return INVALID_LINE;
        string fifth_token = tokens->at(4);

        // the first operand must be a defined vector and must be of the same dimension as the result vector
        if (vectors->count(fifth_token) == 0 || defined_variables->count(fifth_token) == 0) return VAR_REFERENCED_BEFORE_DEFINED; 
        if (vector_dimensions->at(fifth_token) != vector_dimensions->at(second_token)) return VECTORS_OF_DIFFERENT_DIMENSION;

        return 0;

    }

    return INVALID_LINE;

}


bool Preprocessor::is_valid_macro(const string& name) {
    return macros->count(name) > 0;
}

bool Preprocessor::is_vector_component(const string& name) {
    if (name.size() < 3) return false;

    // find the colon
    size_t colon_pos = name.find_first_of(".");
    if (colon_pos + 1 >= name.length()) return false;

    // grab the name of the vector, make sure there is a vector by this name
    string vec_name = name.substr(0, colon_pos);
    if (vectors->count(vec_name) == 0) return false;

    // grab the component number
    // make sure it's a valid component number (is actually an int and is less than the vector's dimension)
    string vec_size = name.substr(colon_pos + 1);
    if (!is_int(vec_size)) return false;

    int dimension = stoi(vec_size);
    if (vector_dimensions->at(vec_name) <= dimension) return false;

    return true;
}


bool Preprocessor::has_defined_components(const string& name) {

    if (vectors->count(name) == 0) return false;
    int dimension = vector_dimensions->at(name);

    string component_name;
    for (int i = 0; i < dimension; i++) {
        component_name = name + "." + to_string(i);
        if (defined_variables->count(component_name) > 0) return true;

    }

    return false;

}
 

bool Preprocessor::all_components_defined(const string& name) {

    if (vectors->count(name) == 0) return false;
    int dimension = vector_dimensions->at(name);

    string component_name;
    for (int i = 0; i < dimension; i++) {
        component_name = get_vector_component_name(name, i);
        if (defined_variables->count(component_name) == 0) {
            return false;
        }
    }

    return true; 
}


string Preprocessor::get_vector_component_name(const string& vec_name, int component_num) {
    return vec_name + "." + to_string(component_num);
}

string Preprocessor::get_intermediate_name(const string& var_name, int intvar_num) {
    return var_name + "." + to_string(intvar_num);
}




