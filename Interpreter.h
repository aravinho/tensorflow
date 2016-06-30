#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include "BindingsDictionary.h"
#include "utilities.h"

using namespace std;


class Interpreter {
	BindingsDictionary bindings;
	unordered_map<string, VariableType>* var_types;

public:

	/* Constructor.
	 * Initializes var_types to be an empty unordered map.
	 */
	Interpreter();

	/* Interprets the program stored in the file with the given filename.
	 * Takes in a "vector" of inputs in the form of an unordered map of {name, value} pairs.
	 * Populates a "vector" of outputs with similar {name, value} pairs.
	 *
	 * Interpreting a program consists of reading it line by line and taking appropriate actions for each line.
	 * For every variable declaration line, a dummy binding is added to the Bindings Dictionary.
	 * For every variable definition line, the value of the variable is evaluated and bound to the variable's name.
	 *
	 * This method returns 0 on success, and the appropriate error code on failure (see utilities.h).
	 */
	int interpret(const string& filename, const unordered_map<string, float>& inputs, unordered_map<string, float> *outputs);

	/* Takes in a line, and takes the appropriate action with regards to the Bindings Dictionary.
	 *
	 * If the line declares a variable, adds a dummy binding.
	 * If the line declares an input or weight, binds the variable name to the correct value from the given vector of inputs.
	 * If the line defines an input or weight, this is an error.
	 * If the line defines any other type of variable, parse the operation and two operands.
	 * Apply the operation to the two operands and bind this resulting value to the variable's name.
	 * If the line defines an output, add this {name, value} pair to the given outputs vector.
	 *
	 * This method returns 0 on success, and the appropriate error code on failure (see utilities.h).
	 */
	int parse_line(char line[], const unordered_map<string, float>& inputs);

};

/* -------------------------- Helper Functions ----------------------------- */


/* Applies the given operation to the given operands and returns the resulting value.
 *
 * If either operand is FLT_MIN, returns FLT_MIN.
 * If either operand is FLT_MAX, returns FLT_MAX.
 * If one operand is FLT_MIN and the other is FLT_MAX, returns FLT_MIN.
 * If the given operation is invalid, returns FLT_MIN.
 */
float apply_operation(OperationType operation, float operand1, float operand2);




#endif
