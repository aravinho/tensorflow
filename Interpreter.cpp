#include <iostream>
#include <unordered_map>
#include <fstream>
#include <cfloat>

#include "utilities.h"
#include "Interpreter.h"

#include <math.h>

using namespace std;


/* ---------------- Constructor --------------- */

Interpreter::Interpreter() {
	var_types = new unordered_map<string, VariableType> ();
    vector_dimensions = new unordered_map<string, int> ();

}


/* ---------------- Main Methods -------------- */

int Interpreter::interpret(const string& filename, const unordered_map<string, float>& inputs, unordered_map<string, float> *outputs) {

	if (invalid_file_name(filename)) {
		cerr << "Could not interpret the file " << filename << endl;
		return OTHER_ERROR;
	}

	ifstream prog(filename);

	// buffer into which we read a line from the program.
	char line[MAX_LINE_LENGTH];

    // indicates whether a line was successfully parsed
    int parse_success;

    // Iterate through all the lines of the program
    // Send the line to be parsed
    while(!prog.eof())
    {
        prog.getline(line, MAX_LINE_LENGTH);
        //cout << "line: " << line << endl;
        parse_success = parse_line(line, inputs);
        if (parse_success != 0) {
            cerr << "Invalid line: " << line << endl;
            return parse_success;
        }
    }

    prog.close();


    // accumulate outputs
    for (unordered_map<string, float>::iterator it = bindings.bindings->begin(); it != bindings.bindings->end(); ++it) {
    	string var_name = it->first;
    	float value = it->second;
    	//cout << "var: " << var_name << ", val: " << value << endl;

    	if (var_types->count(var_name) != 0) {
    		if ((*var_types)[var_name] == VariableType::OUTPUT) {
    			//cout << "********output: " << var_name << ", val: " << value << endl;
    			outputs->insert(make_pair(var_name, value));
    		}
    	}


    }
    

	return 0;
}


int Interpreter::parse_line(char line[], const unordered_map<string, float>& inputs) {
	
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
    int success = 0;

    VariableType vec_type;
    string vec_name;

    // If the line is a declaration of a variable,
    // add the name to the Bindings Dictionary.
    // If it's of type input or weight, bind its value to the appropriate value in the input vector.
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


        // add name to Bindings Dictionary
        success = bindings.add_variable(var_name);
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
        	success = bindings.bind_value(var_name, inputs.at(var_name));
        	if (success == -1) {
        		return OTHER_ERROR;
        	}
        }
       
    } 


    else if (inst_type == InstructionType::DECLARE_VECTOR) {
        
        // grab the variable type for the vector
        v_type = strtok(NULL, " ");
        if (v_type == NULL) return INVALID_LINE;
        
        // make sure the vector's variable type is valid
        vec_type = get_variable_type(string(v_type));
        if (vec_type == VariableType::INVALID_VAR_TYPE) {
            return INVALID_LINE;
        }

        // grab the vector name and make sure it's valid
        v_name = strtok(NULL, " ");
        if (v_name == NULL) return INVALID_LINE;
        vec_name = string(v_name);
        if (invalid_var_name(vec_name)) {
            return INVALID_VAR_NAME;
        }

        // grab the size of the vector and make sure it's a valid int
        char *v_size = strtok(NULL, " ");
        if (v_size == NULL) return INVALID_LINE;
        if (!is_int(string(v_size))) {
            return INVALID_LINE;
        }
        int vec_size = stoi(string(v_size));


    	// add an entry in the vector dimensions dictionary
        vector_dimensions->insert(make_pair(vec_name, vec_size));

    	// create the appropriate dummy variables x.0, x.1 etc
        // if input, weight or exp_output vector, bind to those values
        // otherwise leave as dummy
        for (int i = 0; i < vec_size; i++) {
            string component_name = vec_name + "." + string(v_size);
            success = bindings.add_variable(component_name);
            if (success == -1) {
                return VAR_DECLARED_TWICE;
            }

            if (vec_type == VariableType::INPUT || vec_type == VariableType::WEIGHT || vec_type == VariableType::EXP_OUTPUT) {
                success = bindings.bind_value(component_name, inputs.at(component_name));
                if (success == -1) {
                    return OTHER_ERROR;
                }
            }

        }

        return success;
    	
    }



    // If the line is the definition of a variable:
    // make sure it's not an input, weight, or exp_output
    // make sure it has been declared, but that it hasn't been defined before
    // grab the values of the two operands (one or both may be constants)
    // Apply the operation to the two operands
    else if (inst_type == InstructionType::DEFINE) {

    	// grab the variable name, make sure it exists, and hasn't already been defined
        v_name = strtok(NULL, " ");
        if (v_name == NULL) {
        	return INVALID_LINE;
        }
        var_name = string(v_name);
        if (invalid_var_name(var_name)) {
            return INVALID_VAR_NAME;
        }
        if (!bindings.has_been_declared(var_name)) {
        	return VAR_DEFINED_BEFORE_DECLARED;
        }
        if (bindings.has_been_defined(var_name)) {
        	return VAR_DEFINED_TWICE;
        }


        // make sure the next token is the equal sign
        char *equal_sign = strtok(NULL, " ");
        if (equal_sign == NULL || strcmp(equal_sign, "=") != 0) {
        	return INVALID_LINE;
        }
        char *after_equal = strtok(NULL, " ");

        // The variable might be defined in one of three ways:
        // 1. As a constant
        // 2. As equivalent to the value another variable
        // 3. As a binary operation of two variables/constants
        
        if (after_equal == NULL) {
        	return INVALID_LINE;
        }

        // If the variable is defined as a constant, bind this constant value to the name
        if (is_constant(string(after_equal))) {
        	float constant_value = stof(after_equal, NULL);
        	success = bindings.bind_value(var_name, constant_value);
        	return success;
        }
        // If the variable is defined as an operation of two operands,
        // grab the operator and two operands.
        // if either operand is a variable, grab its value from the Bindings Dictionary.
        // evaluate the expression, and bind the current variable to this value.
        OperationType operation = get_operation_type(string(after_equal));

        if (operation == OperationType::INVALID_OPERATION) {
        	// If the variable isn't defined as a constant or as a function of two operands,
        	// it must be defined as equivalent to another variable,
        	// make sure this second variable has already been defined. 
        	string equiv_var(after_equal);
        	if (!bindings.has_been_defined(equiv_var)) {
        		return VAR_REFERENCED_BEFORE_DEFINED;
        	}
        
        	float equiv_var_value = bindings.get_value(equiv_var);
        	success = bindings.bind_value(var_name, equiv_var_value);
        }

        else if (operation == OperationType::ADD || operation == OperationType::MUL) {

        	char *op1 = strtok(NULL, " ");
        	if (op1 == NULL) {
        		return INVALID_LINE;
        	}

        	char *op2 = strtok(NULL, " ");
        	if (op2 == NULL) {
        		return INVALID_LINE;
        	}

        	float operand1, operand2;

        	if (is_constant(op1)) {
        		operand1 = stof(op1, NULL);
        	} else {
        		if (!bindings.has_been_defined(string(op1))) {
        			return VAR_REFERENCED_BEFORE_DEFINED;
        		}
        		operand1 = bindings.get_value(string(op1));

        	} 
        	if (is_constant(op2)) {
        		operand2 = stof(op2, NULL);
        	} else {
        		if (!bindings.has_been_defined(string(op2))) {
        			return VAR_REFERENCED_BEFORE_DEFINED;
        		}
        		operand2 = bindings.get_value(string(op2));
        	}


        	float new_var_value = apply_operation(operation, operand1, operand2);
        	success = bindings.bind_value(var_name, new_var_value);
        }

        else if (operation == OperationType::LOGISTIC || operation == OperationType::DERIV_LOGISTIC) {
        	char *op = strtok(NULL, " ");
        	if (!op) return INVALID_LINE;

        	float operand;

        	if (is_constant(op)) {
        		operand = stof(op, NULL);
        	} else {
        		if (!bindings.has_been_defined(string(op))) {
        			return VAR_REFERENCED_BEFORE_DEFINED;
        		}
        		operand = bindings.get_value(string(op));
        	}

        	// apply_operation will know to ignore the second operand if the operation is logistic
        	float new_var_value = apply_unary_operation(operation, operand);
        	success = bindings.bind_value(var_name, new_var_value);
        
    	}


    	else if (operation == OperationType::DOT) {
    		// keep a dictionary of vector dimensions
    		// add to this on a declare_vector instruction

            // grab the names of the two vectors being dot producted
            char *op1 = strtok(NULL, " ");
            if (!op1) return INVALID_LINE;
            string operand1(op1);
            char *op2 = strtok(NULL, " ");
            if (!op2) return INVALID_LINE;
            string operand2(op2);

            // look up the dimension of the two operand vectors. make sure theyre equal
            if (vector_dimensions->count(operand1) == 0 || vector_dimensions->count(operand2) == 0) {
                return INVALID_LINE;
            }

            if (vector_dimensions->at(operand1) != vector_dimensions->at(operand2)) {
                return INVALID_LINE;
            }

            int vec_size = vector_dimensions->at(operand1);
    		
    		// in a loop, look up the components of a, put them in a vector
            // if any havent been defined, barf
            // do the same for b
            float vec1[vec_size], vec2[vec_size];

            for (int i = 0; i < vec_size; i++) {
                float component_1 = bindings.get_value(operand1 + "." + to_string(i));
                float component_2 = bindings.get_value(operand2 + "." + to_string(i));

                if (component_1 == FLT_MIN || component_2 == FLT_MIN) {
                    return INVALID_LINE;
                }

                vec1[i] = component_1;
                vec2[i] = component_2;
            }

            // call apply_dot_product(a, b)
            // value bind that value
            float dot_product = apply_dot_product(vec1, vec2, vec_size);
            success = bindings.bind_value(var_name, dot_product);
    		
    		
    	}

    }


	return success;
}




/* ---------------------------------- Helper Functions ------------------------- */

float apply_operation(OperationType operation, float operand1, float operand2) {
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
		return operand1 + operand2;
	}

	if (operation == OperationType::MUL) {
		return operand1 * operand2;
	}


	return FLT_MIN;
}

float apply_unary_operation(OperationType operation, float operand) {
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
        return 1 / (1 + exp(-1 * operand));
    }

    if (operation == OperationType::DERIV_LOGISTIC) {
        return exp(operand) / pow(1 + exp(operand), 2);
    }

    return FLT_MIN;
}

float apply_dot_product(float vec1[], float vec2[], int length) {
    float result = 0;
    for (int i = 0; i < length; i++) {
        result += (vec1[i] * vec2[i]);
    }

    return result;
}
