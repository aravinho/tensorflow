#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include "BindingsDictionary.h"
#include "utilities.h"

using namespace std;


/* The Interpreter executes a TenFlang Program.
 * Given a program and a set of inputs, the Interpreter evaluates the outputs of the program.
 * 
 * The Interpreter works by maintaining a BindingsDictionary.
 * The BindingsDictionary contains mapping from variable names to their values.
 * At every line that declares a variable, a dummy entry is inserted nto the BindingsDictionary.
 * At every line that defines a variable, the variable's value is evaluated.
 * The dummy entry for the variable is updated to store the variable's value.
 *
 * The Interpreter is a key component to the Weight Calculation Phase.
 * The GCP is interpreted repeatedly with different combinations of weights and training data input.
 *
 * Before a program can be interpreted, it must be preprocessed by the Preprocessor.
 * The interpreter can only deal with primitive operations, and not vector operations or user-defined macros.
 */

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
	int parse_line(const string& line, const unordered_map<string, float>& inputs);

};

/* -------------------------- Helper Functions ----------------------------- */


/* Applies the given operation to the given operands and returns the resulting value.
 *
 * If either operand is FLT_MIN, returns FLT_MIN.
 * If either operand is FLT_MAX, returns FLT_MAX.
 * If one operand is FLT_MIN and the other is FLT_MAX, returns FLT_MIN.
 * If the given operation is invalid, returns FLT_MIN.
 */
float apply_binary_operation(OperationType operation, float operand1, float operand2);

/* Applies the given unary operation to the given operand and returns the resulting value.
 *
 * If the operand is FLT_MIN or the operation is invalid, returns FLT_MIN.
 * If the operand is FLT_MAX, returns FLT_MAX.
 * If the given operation is invalid, returns FLT_MIN.
 */
float apply_unary_operation(OperationType operation, float operand);



#endif
