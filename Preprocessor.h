#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;


/* These delimiters separate tokens in a line of TenFlang code. 
 */
const char delimiters[3] = " \t";

struct macro {
	string name;
	int num_lines;
	bool is_binary;
	string result;
	string operand1;
	string operand2;
	vector<string> *lines;
};


class Preprocessor {

	

public:
	/* Maps variable names to their types. */
	unordered_map<string, VariableType> *variables;
	/* Maps vector names to their types. */
	unordered_map<string, VariableType> *vectors;
	/* Maps vector names to their dimensions. */
	unordered_map<string, int> *vector_dimensions;
	/* A set of which variables have been defined. */
	unordered_set<string> *defined_variables;
	/* Maps macro names to their definitions. */
	unordered_map<string, struct macro>* macros;

	/* Theis boolean is used to ensure all the macro definitions occur at the top. */
	bool macros_done;

	/* Constructor.
     * Initializes the instance variables.
     */
    Preprocessor();

	/* Before a program can be compiled or interpreted, it must be expanded.
	 * The preprocessor expands a program by breaking complex instructions into simpler primitives.
	 * Instructions that require expanding are:
	 * 	1. DECLARE_VECTOR instructions, 2. DEFINE instructions with vector operations, and 3. DEFINE instructions with macros.
	 * 
     * The declare_vector instruction gets broken down into as many regular declare instructions as there are elements in the vector.
     *
     * ex. "declare_vector input x[3]" becomes:
     * "declare input x.1"
     * "declare input x.2"
     * "declare input x.3"
     *
     * Defintions of variables as vector operations on vector variables get broken down into component-wise primitives.
     * For example, the dot product of two vectors get broken up into several multiplications and additions.
     *
     * ex. "define_dot dot_prod = dot a b" becomes the following: (Assume dot_prod, a and b have been declared, and a and b are both two-element vectors.)
     * "declare dot_prod.1"
     * "declare dot_prod.2"
     * "define dot_prod.1 = mul a.1 b.1"
     * "define dot_prod.2 = mul a.2 b.2"
     * "define dot_prod = add dot_prod.1 dot_prod.2"
     *
     * This method reads from the Program, and writes to an Expanded Program file.
     * The second pass will then compile the GCP from the Expanded Shape Program.
     *
     * Returns 0 on success, and the appropriate error code otherwise (see utilities.h).
     */
    int expand_program(const string& shape_prog_filename, const string& expanded_shape_prog_filename);


    /* Populates the expanded_prog_lines array with the appropriate expansion of prog_line.
     * See the comment for the expand_program for details on what "expansion" entails.
     * If no expansion is needed, then the line is copied directly into the zeroth string of the expanded_prog_lines array.
     *
     * Returns the number of lines copied into expanded_prog_lines_array.
     * This is 1 if no expansion was needed, and the appropriate error code (see utilities.h) if the given line was invalid.
     * Returns 0 if prog_line is an empty line.
     */
    int expand_line(char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH], char prog_line[]); 




    /* Determines whether the given tokens form a valid DECLARE instruction.
     * In order to be valid, there must be exactly 3 tokens.
     * The first must be the word "declare", the second a valid variable type, and the third a valid variable name.
     * See the invalid_var_name function in utilities.h for what constitutes a valid variable name.
     * This must be the first declaration of this variable name.
     *
     * Returns 0 if the instruction is valid, or an error code otherwise (see utilities.h).
     */
    int is_valid_declare_line(char *tokens[], int num_tokens);

    /* Determines whether the given tokens form a valid DEFINE instruction.
     * In order to be valid, there must be exactly 4 tokens.
     * The first must be the word "declare_vector" and the second a valid variable type.
     * The third must be a valid variable name and the fourth an int (the dimension of the vector).
     * The size of the vector must be a positive integer and cannot exceed MAX_VECTOR_SIZE (defined in utilities.h).
     * This must be the first declaration of this variable name.
     * See the invalid_var_name function in utilities.h for what constitutes a valid variable name.
     *
     * Returns 0 if the instruction is valid, or an error code otherwise (see utilities.h).
     */
    int is_valid_declare_vector_line(char *tokens[], int num_tokens);

    /* Determines whether the given tokens form a valid DEFINE instruction.
     * A variable must be declared before it is defined, and cannot be defined twice.
     * If a variable is defined as an operation of one or more other variables, all these other variables must be defined.
     * Input, weight and expected output variables cannot be defined.
     * 
     * Every DEFINE instruction begins with "define <var_name> = "
     * A variable can be defined in one of five ways:
     *
     * 1. "define <var_name> = <primitive_operation> <operand 1> <operand 2>"
     *  - if either operand is a variable, it must have been defined.
     *
     * 2. "define <var_name> = <user-defined macro operation> <operand 1> <operand 2>"
     *  - The macro must have been previously defined, the same rules apply as for primitive operations.
     *
     * 3. "define <var_name> = <vector operation> <operand 1> <operand 2>"
     * 	- for binary vector operations, both operand vectors and the result vector must be of the same dimension.
     *
     * 4. "define <var_name> = <constant>"
     * 
     * 5. "define <var_name> = <another variable with a different name>"
     *
     * This method returns 0 if the instruction is valid, or an error code otherwise (see utilities.h)
     */
    int is_valid_define_line(char *tokens[], int num_tokens);

    /* Expands a DECLARE_VECTOR instruction, copying the expanded lines into the EXPANDED_PROG_LINES array.
     * The declare_vector instruction gets broken down into as many regular declare instructions as there are elements in the vector.
     *
     * ex. "declare_vector input x[3]" becomes:
     * "declare input x.1"
     * "declare input x.2"
     * "declare input x.3"
     *
     * This method returns the dimension of the vector, which will always be a positive integer.
     */
    int expand_declare_vector_instruction(char *tokens[], char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]);

    /* Expands a DEFINE instruction, copying the expanded lines into the EXPANDED_PROG_LINES array.
     * DEFINE instructions need expanding if they involve vector operations or user-defined macros.
     * Vector operations are expanded into component wise operations.
     * Macro instructions are expanded directly based on the user given macro.
     *
     * If the instruction requires no expanding, nothing is done and 0 is returned.
     * Otherwise, this method returns the number of expanded lines.
     */
    int expand_define_instruction(char *tokens[], int num_tokens, char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]);

    int expand_unary_macro(const string& macro_name, const string& operand, const string& result, 
    	char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]);


	int expand_binary_macro(const string& macro_name, const string& operand1, const string& operand2, const string& result, 
    	char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]);


	int expand_vector_instruction(const OperationType& oper_type, const string& result_vec, const string& operand1, const string& operand2,
    	char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]);

	int expand_dot_product_instruction(const string& result_vec, const string& vector1, const string& vector2, int dimension,
    char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]);

	int expand_component_wise_add_instruction(const string& result_vec, const string& vec1, const string& vec2, int dimension, 
    	char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]);

	int expand_component_wise_mul_instruction(const string& result_vec, const string& vec1, const string& vec2, int dimension, 
    	char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]);

	int expand_scale_vector_instruction(const string& result_vec, const string& vec, const string& constant, int dimension, 
    	char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]);

	int expand_increment_vector_instruction(const string& result_vec, const string& vec, const string& constant, int dimension, 
    	char expanded_prog_lines[MAX_EXPANSION_FACTOR][MAX_LINE_LENGTH]);

	bool is_valid_macro(const string& name);

	bool is_binary_macro(const string& name);

	bool is_unary_macro(const string& name);


	string substitute_dummy_names(const string& dummy_line, string dummy_result, string dummy_op1, string dummy_op2,
    	const string& result, const string& operand1, const string& operand2);

	int parse_macro_line(char macro_line_copy[], struct macro *macro);

	int parse_macro_subsequent_line(char line[], struct macro *macro);

	int parse_macro_first_line(char first_line[], struct macro *macro);

};


/* Populates the tokens array with the tokens of LINE.
 * Tokens are delimited by any character in the given DELIM string.
 * Returns the number of tokens, or an error code on failure (see utilities.h).
 *
 * This function uses the strtok method to tokenize.
 * As a result, the given char-pointer is mangled during the execution of this function.
 */
int tokenize_line(char line[], char *tokens[MAX_NUM_TOKENS], const string& delim);



#endif
