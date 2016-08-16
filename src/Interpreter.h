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

	BindingsDictionary *bindings;
	unordered_map<string, VariableType>* var_types;

public:

	/* Constructor.
	 * Initializes var_types to be an empty unordered map.
	 * Initializes the Bindings Dictionary.
	 */
	Interpreter();

	/* Destructor.
	 * Deletes the var_types map, and the Bindings Dictionary.
	 */
	~Interpreter();

	/* Interprets the program stored in the file with the given FILENAME.
	 * First Parses input name-value pairs from the given file INPUT_FILENAME, by calling parse_input_file.
	 * parse_input_file builds up a map of inputs from the file.
	 * This method then passes this map to the "interpret" function.
	 * "interpret" accumulates the outputs of the program.
	 * This function then prints out these outputs in the same {<var_name>	<value>} format.
	 * 
	 * Returns 0 on success, or an error code on failure (see utilities.h).
	 * An error might occur in the parsing of the input file, or during interpretation of the program.
	 */
	int interpret_program(const string& filename, const string& input_filename);

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
	int interpret(const string& filename, const unordered_map<string, double>& inputs, unordered_map<string, double> *outputs);

	/* Parses input name-value pairs from the given file INPUT_FILENAME.
	 * Writes these name-value pairs into the given INPUT_MAP.
	 * The input file is made of {<var_name>	<value> pairs}, with a tab separating the name and value.
	 * An input file might look like this:
	 * 
	 	x.0		3
	  	x.1		-0.2
	  	w.2		0.9
	 	w.1		0.7
	 	y.0		8
	 	y.1		-4.3
	 *
	 * This method calls parse_input_line for every line of the file.
	 * Upon completion of this method, the map of input name-value pairs is built.
	 * Returns 0 on success, or an error code on failure (see utilities.h).
	 */
	int parse_input_file(const string& input_filename, unordered_map<string, double> *input_map);

	/* Takes in a line of an input file, and extracts the input variable's name and value.
	 * The given pointers INPUT_VAR_NAME and INPUT_VAR_VALUE are written to accordingly.
	 * Recall that an input file line looks like: "<var_name>	<value>".
	 *
	 * Returns 0 on success, or an error code if the line is invalid.
	 * A line can be invalid if it has more or fewer than 2 tokens,
	 *	or if the second token (var value) cannot be parsed as a double.
	 * If an input line defines a variable that a previous line has defined, 
	 *	intepret_program will catch this error.
	 */
	int parse_input_line(const string& input_line, string *input_var_name, double *input_var_value);

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
	int parse_line(const string& line, const unordered_map<string, double>& inputs);

	/* Iterates through all the variables in the BindingsDictionary.
	 * If a variable is an OUTPUT variable,
	 *	adds the name-value pair to the given map of outputs.
	 *
	 * This method is called after every line has been parsed.
	 */
	void accumulate_outputs(unordered_map<string, double> *outputs);

	/* Returns the BindingsDictionary of this Interpreter. */
	BindingsDictionary *get_bindings_dictionary();

	/* Returns the VAR_TYPES map of this Interpreter. */
	unordered_map<string, VariableType> *get_var_types();


};

/* -------------------------- Helper Functions ----------------------------- */


/* Applies the given operation to the given operands and returns the resulting value.
 *
 * If either operand is DBL_MIN, returns DBL_MIN.
 * If either operand is DBL_MAX, returns DBL_MAX.
 * If one operand is DBL_MIN and the other is DBL_MAX, returns DBL_MIN.
 * If the given operation is invalid, returns DBL_MIN.
 */
double apply_binary_operation(OperationType operation, double operand1, double operand2);

/* Applies the given unary operation to the given operand and returns the resulting value.
 *
 * If the operand is DBL_MIN or the operation is invalid, returns DBL_MIN.
 * If the operand is DBL_MAX, returns DBL_MAX.
 * If the given operation is invalid, returns DBL_MIN.
 */
double apply_unary_operation(OperationType operation, double operand);



#endif
