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
    string prog_line;

    // indicates how many lines were generated from a given line in the Shape Program.
    // 1 if no expansion was necessary, 0 for an empty line, -1 if the Shape Program line was invalid.
    int num_lines_expanded;

    // expand each line
    while(!prog.eof()) {

        getline(prog, line);
        num_lines_expanded = expand_line(prog_line, exp_prog);
        if (num_lines_expanded < 0) return num_lines_expanded;

    }

    prog.close();
    exp_prog.close();

    return 0;

}


int Preprocessor::expand_line(const string& prog_line, ofstream& exp_prog) {
    
    // edge error cases
    if (prog_line == NULL) return OTHER_ERROR;
    if (prog_line.compare("") == 0) return 0;

    // tokenize the line
    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(line, tokens, " ");
    if (num_tokens < 3) return INVALID_LINE;

    // grab the instruction type, and act accordingly
    if (!is_valid_instruction(tokens->at(0)) return INVALID_LINE;
    InstructionType inst_type = get_instruction_type(tokens->at(0));


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
    
    // as soon as we see a non-macro-definition, there can be no more macros
    macros_done = true;

    if (inst_type == InstructionType::DECLARE) {

        // verify the line is a valid DECLARE instruction
        int is_valid_declare_line = is_valid_declare_line(prog_line);
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

        // the number of expanded lines is precisely the dimension of the vector (one line per component)
        return vector_size;

    }

    return INVALID_LINE;

}


/* ------------------------- Main Expansion Methods ------------------------- */


int Preprocessor::expand_declare_vector_instruction(const string& line, ofstream &exp_prog) {

    if (line.compare("")) return 0;

    // tokenize the line
    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(line, tokens);
    if (num_tokens != 4) return INVALID_LINE;

    // grab the vector name, type and size
    string vec_type = tokens->at(1), vec_name = tokens->at(2);
    int vec_size = stoi(tokens->at(3));
    
    // expand into declarations of components
    for (int i = 0; i < vec_size; i++) {
        exp_prog << "declare " << vec_type << " " << vec_name << "." << i << endl;
    }

    return vec_size;    
}


int Preprocessor::expand_define_instruction(const string& line, ofstream &exp_prog) {

    if (line.compare("")) return 0;

    // tokenize the line
    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(line, tokens);
    if (num_tokens < 4 || num_tokens > 6) return INVALID_LINE;

    string var_name = tokens->at(1), operation = tokens->at(3);

    if (is_valid_vector_operation(operation)) {
        OperationType vec_oper = get_operation_type(operation);
        string operand1 = tokens->at(4);
        string operand2 = tokens->at(5);
        return expand_vector_instruction(vec_oper, var_name, operand1, operand2, exp_prog);

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

    else if (is_constant(operation) || is_valid_var_name(operation)) {
        exp_prog << line << endl;
        return 1;
    }

    return INVALID_LINE;

}


int Preprocessor::expand_vector_instruction(const OperationType& oper_type, const string& result, const string& operand1, const string& operand2,
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
    macro->num_references++;

    for (int i = 0; i < macro->num_lines; i++) {
        string dummy_line = macro_lines->at(i);
        string modified_line = substitute_dummy_names(dummy_line, macro->result, macro->operand1, "", result, operand, "", macro->num_references);
        exp_prog << modified_line << endl;
    }

    return macro->num_lines;

}


int Preprocessor::expand_binary_macro(const string& macro_name, const string& operand1, const string& operand2, const string& result, 
    ofstream& exp_prog) {

    struct macro *macro = macros->at(macro_name);
    vector<string> *macro_lines = macro->lines;
    macro->num_references++;

    for (int i = 0; i < macro->num_lines; i++) {
        string dummy_line = macro_lines->at(i);
        string modified_line = substitute_dummy_names(dummy_line, macro->result, macro->operand1, macro->operand2, result, operand1, operand2, macro->num_references);
        exp_prog << modified_line << endl;
    }

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
            modified_line.append(result + " ");
        }
        else if (token.compare(dummy_op1) == 0) {
            modified_line.append(operand1 + " ");
        }
        else if (token.compare(dummy_op2) == 0) {
            modified_line.append(operand2 + " ");
        }
        else if (!is_keyword(token) && !is_constant(token)) {
            modified_line.append(token + to_string(macro_num_references) + " ");
        }
        else {
            modified_line.append(token + " ");
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
    vector<string> sub_macro_lines = new vector<string>();
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
    vector<string> tokens = new vector<string>();
    int num_tokens = tokenize_line(macro_line, tokens, " ");

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


int Preprocessor::parse_macro_subsequent_line(const string& line, struct macro *macro) {
    
    if (macro == NULL) return OTHER_ERROR;
    if (line.compare("") == 0) return 0;

    // tokenize
    vector<string> tokens = new vector<string>();
    int num_tokens = tokenize_line(macro_line, tokens, " ");
    
    // temporarily declare or define this variable
    if (is_valid_declare_line(line) == 0) {
        string var_name = tokens->at(2);
        VariableType var_type = get_variable_type(tokens->at(1));
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
    char a[1000], b[1000], c[1000], d[1000], e[1000], f[1000], g[1000];
    strcpy(a, "#macro res = my_macro opone optwo; declare intvar lala; define lala = logistic opone; declare intvar q; define q = pow lala 3; define res = mul optwo q");
    strcpy(b, "declare input a");
    strcpy(c, "declare weight b");
    strcpy(d, "declare output c");
    strcpy(e, "define c = my_macro a b");
    strcpy(f, "declare intvar g");
    strcpy(g, "define g = my_macro a b");
    Preprocessor *p = new Preprocessor();
    test_expand(a, p);
    test_expand(b, p);
    test_expand(c, p);
    test_expand(d, p);
    test_expand(e, p);
    test_expand(f, p);
    test_expand(g, p);
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




