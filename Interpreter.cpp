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

int Interpreter::interpret(const string& filename, const unordered_map<string, float>& inputs, unordered_map<string, float> *outputs) {

	if (!is_valid_file_name(filename)) {
		cerr << "Could not interpret the file " << filename << endl;
		return OTHER_ERROR;
	}

	ifstream prog(filename);

	// buffer into which we read a line from the program.
    string line;

    // indicates whether a line was successfully parsed
    int parse_success;

    // Iterate through all the lines of the program
    // Send the line to be parsed
    while(!prog.eof())
    {
        getline(prog, line);
        parse_success = parse_line(line, inputs);
        if (parse_success != 0) {
            cerr << "Invalid line: " << line << endl;
            return parse_success;
        }
    }

    prog.close();


    // accumulate outputs
    accumulate_outputs(outputs);
	return 0;
}


int Interpreter::parse_line(const string& line, const unordered_map<string, float>& inputs) {
	
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
        	float constant_value = stof(tokens->at(3), NULL);
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
        
        	float equiv_var_value = bindings->get_value(equiv_var);
        	success = bindings->bind_value(var_name, equiv_var_value);
            return success;
        }


        // If the variable is defined as an operation of two operands,
        // grab the operator and two operands.
        // if either operand is a variable, grab its value from the Bindings Dictionary.
        // evaluate the expression, and bind the current variable to this value.
        if (is_binary_primitive(tokens->at(3))) {

            if (num_tokens != 6) return INVALID_LINE;

        	float operand1, operand2;

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

        	float new_var_value = apply_binary_operation(operation, operand1, operand2);
        	success = bindings->bind_value(var_name, new_var_value);
            return success;
        }

        if (is_unary_primitive(tokens->at(3))) {

            if (num_tokens != 5) return INVALID_LINE;

            float operand1;

            if (is_constant(tokens->at(4))) {
                operand1 = stof(tokens->at(4), NULL);
            } else {
                if (!bindings->has_been_defined(tokens->at(4))) return VAR_REFERENCED_BEFORE_DEFINED;
                operand1 = bindings->get_value(tokens->at(4));

            }
        	
        	float new_var_value = apply_unary_operation(operation, operand1);
        	success = bindings->bind_value(var_name, new_var_value);
            return success;
        
    	}

    	
    }


	return INVALID_LINE;
}


/* ---------------------------------- Helper Functions ------------------------- */

float apply_binary_operation(OperationType operation, float operand1, float operand2) {

    float result = FLT_MIN;

	if (operation == OperationType::INVALID_OPERATION) {
		return FLT_MIN;
	}

	if (operand1 == FLT_MIN || operand2 == FLT_MIN) {
		return FLT_MIN;
	}

	if (operand1 == FLT_MAX || operand2 == FLT_MAX) {
		return FLT_MAX;
	}

	if (operation == OperationType::ADD) {
		result = operand1 + operand2;
	}

	if (operation == OperationType::MUL) {
		result = operand1 * operand2;
	}

    if (operation == OperationType::POW) {
        if (operand1 == 0 && operand2 < 0) {
            cerr << "Divide by zero error." << endl;
            return FLT_MIN;
        }
        result = pow(operand1, operand2);
    } 

	return isnan(result) || result == FLT_MIN ? FLT_MIN : result;
}


float apply_unary_operation(OperationType operation, float operand) {

    float result = FLT_MIN;

    if (operation == OperationType::INVALID_OPERATION) {
        return FLT_MIN;
    }

    if (operand == FLT_MIN) {
        return FLT_MIN;
    }

    if (operand == FLT_MAX) {
        return FLT_MAX;
    }

    if (operation == OperationType::LOGISTIC) {
        result = 1 / (1 + exp(-1 * operand));
    }

    if (operation == OperationType::EXP) {
        result = exp(operand);
    }

    if (operation == OperationType::LN) {
        if (operand <= 0) {
            cerr << "Cannot take the log of a non-positive number." << endl;
            return FLT_MIN;
        }
        result = log(operand);
    }

    return isnan(result) || result == FLT_MIN ? FLT_MIN: result;
}

void Interpreter::accumulate_outputs(unordered_map<string, float> *outputs) {

    for (unordered_map<string, float>::iterator it = bindings->bindings->begin(); it != bindings->bindings->end(); ++it) {
        
        string var_name = it->first;
        float value = it->second;

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
