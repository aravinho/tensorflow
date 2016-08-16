#include <iostream>
#include <unordered_map>
#include <fstream>
#include <cfloat>

#include "utilities.h"
#include "Interpreter.h"

#include <math.h>

using namespace std;


/* ---------------- Constructor/Destructor --------------- */

Interpreter::Interpreter() {
	var_types = new unordered_map<string, VariableType> ();
    bindings = new BindingsDictionary();

}


Interpreter::~Interpreter() {
    delete var_types;
    delete bindings;
}


/* ---------------- Main Methods -------------- */

int Interpreter::interpret_program(const string& filename, const string& input_filename) {

    // call parse_input_file which builds up the input map
    unordered_map<string, double> input_map;
    int parse_input_file_success = parse_input_file(input_filename, &input_map);
    // if there was an error parsing the input file, exit
    if (parse_input_file_success != 0) {
        return parse_input_file_success;
    }
    
    // once the input map has been built, initialize an empty map of outputs, and interpret the program
    unordered_map<string, double> *output_map = new unordered_map<string, double>();
    int interpret_success = interpret(filename, input_map, output_map);
    if (interpret_success != 0) {
        return interpret_success;
    }

    for (unordered_map<string, double>::iterator it = output_map->begin(); it != output_map->end(); ++it) {
        cout << it->first << "\t" << it->second << endl;
    }

    return 0;

}

int Interpreter::interpret(const string& filename, const unordered_map<string, double>& inputs, unordered_map<string, double> *outputs) {

	if (!is_valid_file_name(filename)) {
		cerr << "\nCould not interpret the file " << filename << endl << endl;
		return OTHER_ERROR;
	}

	ifstream prog(filename);

	// buffer into which we read a line from the program.
    string line;

    // indicates whether a line was successfully parsed
    int parse_success;

    // Iterate through all the lines of the program
    // Send the line to be parsed
    int line_num = 0;
    while(!prog.eof())
    {
        getline(prog, line);
        // parse this line of the program
        parse_success = parse_line(line, inputs);
        // if there was an error with this line of the program,
        // print the error message and exit
        if (parse_success != 0) {
            cerr << "\nERROR, Line " << line_num << ":" << endl;
            cerr << line << endl;
            cerr << get_error_message(parse_success) << endl << endl;
            prog.close();
            return parse_success;
        }

        line_num++;
    }

    prog.close();


    // accumulate outputs
    accumulate_outputs(outputs);
	return 0;
}


int Interpreter::parse_input_file(const string& input_filename, unordered_map<string, double> *input_map) {

    if (!is_valid_file_name(input_filename)) {
        cerr << "\nCould not open the input file " << input_filename << endl << endl;
        return OTHER_ERROR;
    }

    ifstream input_file(input_filename);
    string input_line;
    int parse_input_line_success = 0;

    // parse the input file, building up the input map of name-value pairs
    int line_num = 0;
    string input_var_name;
    double input_var_value;
    while (!input_file.eof()) {

        getline(input_file, input_line);
        // parse this line of the input file
        parse_input_line_success = parse_input_line(input_line, &input_var_name, &input_var_value);
        // if there is an error with this line of the input file:
        // print the error message and exit
        if (parse_input_line_success != 0) {
            cerr << "\nERROR WITH INPUTS, Line " << line_num << ":" << endl;
            cerr << input_line << endl;
            cerr << get_error_message(parse_input_line_success) << endl << endl;
            input_file.close();
            return parse_input_line_success;
        }

        // if the line is an empty line, continue
        if (input_var_name == "") {
            line_num++;
            continue;
        }

        // if the input line was successfully parsed, add this pair to the map of inputs
        // make sure this variable hasn't already been seen
        if (input_map->count(input_var_name) != 0) {
            cerr << "\nERROR WITH INPUTS, Line " << line_num << ":" << endl;
            cerr << input_line << endl;
            cerr << get_error_message(VAR_DEFINED_TWICE) << endl << endl;
            input_file.close();
            return VAR_DEFINED_TWICE;
        }
        input_map->insert(make_pair(input_var_name, input_var_value));

        line_num++;
    }

    input_file.close();
    return 0;
}


int Interpreter::parse_input_line(const string& input_line, string *input_var_name, double *input_var_value) {
    
    if (input_line == "") {
        *input_var_name = "";
        *input_var_value = 0;
        return 0;
    }

    vector<string> *tokens = new vector<string>();
    int num_tokens = tokenize_line(input_line, tokens, "\t");
    if (num_tokens != 2) {
        if (num_tokens < 0) return num_tokens;
        return INVALID_LINE;
    }

    // make sure the given value can actually be parsed as a float
    if (!is_constant(tokens->at(1))) {
        return INVALID_LINE;
    }

    *input_var_name = tokens->at(0);
    *input_var_value = stod(tokens->at(1));

    return 0;
}


int Interpreter::parse_line(const string& line, const unordered_map<string, double>& inputs) {
	
    if (line == "") return 0;

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
    int success = 0;

    // If the line is a declaration of a variable,
    // add the name to the Bindings Dictionary.
    // If it's of type input or weight, bind its value to the appropriate value in the input vector.
    if (inst_type == InstructionType::DECLARE) {

        if (num_tokens != 3) return INVALID_LINE;

    	// grab the variable type
        var_type = get_variable_type(tokens->at(1));
        if (var_type == VariableType::INVALID_VAR_TYPE) return INVALID_LINE;

        // grab the variable name
        var_name = tokens->at(2);
        if (!is_valid_expanded_var_name(var_name)) return INVALID_VAR_NAME;


        // add name to Bindings Dictionary
        success = bindings->add_variable(var_name);
        if (success == -1) {
            return VAR_DECLARED_TWICE;
        }

        // record the type of this variable
        (*var_types)[var_name] = var_type;

        // if input or weight, bind the name to its value
        if (var_type == VariableType::INPUT || var_type == VariableType::WEIGHT || var_type == VariableType::EXP_OUTPUT) {
        	if (inputs.count(var_name) == 0) {
                return INPUT_VALUE_NOT_PROVIDED;
            }
        	success = bindings->bind_value(var_name, inputs.at(var_name));
        	if (success == -1) return OTHER_ERROR;

        }

        return success;
       
    } 

    // If the line is the definition of a variable:
    // make sure it's not an input, weight, or exp_output
    // make sure it has been declared, but that it hasn't been defined before
    // grab the values of the two operands (one or both may be constants)
    // Apply the operation to the two operands
    else if (inst_type == InstructionType::DEFINE) {

        if (num_tokens < 4) return INVALID_LINE;

    	// grab the variable name, make sure it exists, and hasn't already been defined
        var_name = tokens->at(1);
        if (!is_valid_expanded_var_name(var_name)) return INVALID_VAR_NAME;
        if (!bindings->has_been_declared(var_name)) return VAR_DEFINED_BEFORE_DECLARED;
        if (bindings->has_been_defined(var_name)) return VAR_DEFINED_TWICE;


        // The variable might be defined in one of three ways:
        // 1. As a constant
        // 2. As equivalent to the value another variable
        // 3. As an operation of one or two variables/constants


        // If the variable is defined as a constant, bind this constant value to the name
        if (is_constant(tokens->at(3))) {
            if (num_tokens != 4) return INVALID_LINE;
        	double constant_value = stof(tokens->at(3), NULL);
        	success = bindings->bind_value(var_name, constant_value);
        	return success;
        }

        
        // To see if the variable is defined as equivalent to another variable,
        // check whether the operation type is undefined
        OperationType operation = get_operation_type(string(tokens->at(3)));

        // If the variable isn't defined as a constant or as a function of two operands,
        // it must be defined as equivalent to another variable,
        // make sure this second variable has already been defined. If so, bind its value to this variable's name
        if (operation == OperationType::INVALID_OPERATION) {
        	if (num_tokens != 4) return INVALID_LINE;

        	string equiv_var(tokens->at(3));
        	if (!bindings->has_been_defined(equiv_var)) return VAR_REFERENCED_BEFORE_DEFINED;
        
        	double equiv_var_value = bindings->get_value(equiv_var);
        	success = bindings->bind_value(var_name, equiv_var_value);
            return success;
        }


        // If the variable is defined as an operation of two operands,
        // grab the operator and two operands.
        // if either operand is a variable, grab its value from the Bindings Dictionary.
        // evaluate the expression, and bind the current variable to this value.
        if (is_binary_primitive(tokens->at(3))) {
           
            if (num_tokens != 6) return INVALID_LINE;

        	double operand1, operand2;

        	if (is_constant(tokens->at(4))) {
        		operand1 = stof(tokens->at(4), NULL);
        	} else {
        		if (!bindings->has_been_defined(tokens->at(4))) return VAR_REFERENCED_BEFORE_DEFINED;
        		operand1 = bindings->get_value(string(tokens->at(4)));

        	}

        	if (is_constant(tokens->at(5))) {
        		operand2 = stof(tokens->at(5), NULL);
        	} else {
        		if (!bindings->has_been_defined(tokens->at(5))) return VAR_REFERENCED_BEFORE_DEFINED;
        		operand2 = bindings->get_value(string(tokens->at(5)));
        	}

        	double new_var_value = apply_binary_operation(operation, operand1, operand2);
        	success = bindings->bind_value(var_name, new_var_value);
            return success;
        }

        if (is_unary_primitive(tokens->at(3))) {

            if (num_tokens != 5) return INVALID_LINE;

            double operand1;

            if (is_constant(tokens->at(4))) {
                operand1 = stof(tokens->at(4), NULL);
            } else {
                if (!bindings->has_been_defined(tokens->at(4))) return VAR_REFERENCED_BEFORE_DEFINED;
                operand1 = bindings->get_value(tokens->at(4));

            }
        	
        	double new_var_value = apply_unary_operation(operation, operand1);
        	success = bindings->bind_value(var_name, new_var_value);
            return success;
        
    	}

    	
    }


	return INVALID_LINE;
}


/* ---------------------------------- Helper Functions ------------------------- */

double apply_binary_operation(OperationType operation, double operand1, double operand2) {

    double result = DBL_MIN;

	if (operation == OperationType::INVALID_OPERATION) {
		return DBL_MIN;
	}

	if (operand1 == DBL_MIN || operand2 == DBL_MIN) {
		return DBL_MIN;
	}

	if (operand1 == DBL_MAX || operand2 == DBL_MAX) {
		return DBL_MAX;
	}

	if (operation == OperationType::ADD) {
		result = operand1 + operand2;
	}

    if (operation == OperationType::SUB) {
        return operand1 - operand2;
    }

	if (operation == OperationType::MUL) {
		result = operand1 * operand2;
	}

    if (operation == OperationType::POW) {
        if (operand1 == 0 && operand2 < 0) {
            cerr << "Cannot divide by zero" << endl;
            return DBL_MIN;
        }
        result = pow(operand1, operand2);
    } 

	return isnan(result) || result == DBL_MIN ? DBL_MIN : result;
}


double apply_unary_operation(OperationType operation, double operand) {

    double result = DBL_MIN;

    if (operation == OperationType::INVALID_OPERATION) {
        return DBL_MIN;
    }

    if (operand == DBL_MIN) {
        return DBL_MIN;
    }

    if (operand == DBL_MAX) {
        return DBL_MAX;
    }

    if (operation == OperationType::LOGISTIC) {
        result = 1 / (1 + exp(-1 * operand));
    }

    if (operation == OperationType::EXP) {
        result = exp(operand);
    }

    if (operation == OperationType::LN) {
        if (operand <= 0) {
            cerr << "Cannot take the log of the negative number " << operand << endl;
            return DBL_MIN;
        }
        result = log(operand);
    }

    return isnan(result) || result == DBL_MIN ? DBL_MIN: result;
}

void Interpreter::accumulate_outputs(unordered_map<string, double> *outputs) {

    for (unordered_map<string, double>::iterator it = bindings->bindings->begin(); it != bindings->bindings->end(); ++it) {
        
        string var_name = it->first;
        double value = it->second;

        if (var_types->count(var_name) != 0) {
            if ((*var_types)[var_name] == VariableType::OUTPUT) {
                outputs->insert(make_pair(var_name, value));
            }
        }

    }
    
}


BindingsDictionary *Interpreter::get_bindings_dictionary() {
    return this->bindings;
}


unordered_map<string, VariableType> *Interpreter::get_var_types() {
    return this->var_types;
}
