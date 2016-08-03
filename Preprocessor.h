#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "stdlib.h"

#include "utilities.h"

using namespace std;


/* These delimiters separate tokens in a line of TenFlang code. 
 */
const string delimiters = " \t";

/* Represents a user-defined macro. */
struct macro {
	string name;
	int num_lines;
	bool is_binary;
	string result;
	string operand1;
	string operand2;
	vector<string> *lines;
	int num_references;
};



/* The Preprocessor serves three main functions:
 * 1. Parse user-defined macros
 * 2. Expand a TenFlang program into primitive operations
 * 3. Ensure there are no errors in the program
 *
 * The Preprocessor iterates over all the lines in the TenFlang program.
 * If a line is a definition of a user-defined macro, it is parsed (see parse_macro_line).
 * Otherwise, the Preprocessor makes sure the line is a valid, legal instruction, and expands it if necessary.
 *
 * Lines that require expansions are:
 *	- lines involving vector operations
 *	- lines involving vector operations
 *	- lines involving user-defined macros
 *
 * The Preprocessor is used before a TenFlang program can be compiled or interpreted.
 * This is so that the compiler/interpreter only need deal with simple primitives.
 * After the Preprocessor expands the program, it writes the expanded program to a temp file.
 * This program that can be easily compiled or interpreted.
 */
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
	unordered_map<string, struct macro*>* macros;

	/* This boolean is used to ensure all the macro definitions occur at the top. */
	bool macros_done;



	/* Constructor.
     * Initializes the instance variables.
     */
    Preprocessor();

    /* Destructor.
     * Frees all the member maps and sets.
     * Frees all the macro structs within the macros map.
     */
    ~Preprocessor();


    /* ------------------------------------- Main Methods -------------------------------------- */



	/* Before a program can be compiled or interpreted, it must be expanded.
	 * The preprocessor expands a program by breaking complex instructions into simpler primitives.
	 * Instructions that require expanding are:
	 * 	1. DECLARE_VECTOR instructions, 2. DEFINE instructions with dot product operations, 3. DEFINE instructions with macros, 4. DEFINE_VECTOR_INSTRUCTIONS.
	 * 
     * The declare_vector instruction gets broken down into as many regular declare instructions as there are elements in the vector.
     *
     * ex. "declare_vector input x 3" becomes:
     * "declare input x.1"
     * "declare input x.2"
     * "declare input x.3"
     *
     * Defintions of variables as the dot product of two vector variables get broken down into component-wise primitives.
     * The dot product of two vectors get broken up into several multiplications and additions.
     *
     * ex. "define_dot dot_prod = dot a b" becomes the following: (Assume dot_prod, a and b have been declared, and a and b are both two-element vectors.)
     * "declare dot_prod.1"
     * "declare dot_prod.2"
     * "define dot_prod.1 = mul a.1 b.1"
     * "define dot_prod.2 = mul a.2 b.2"
     * "define dot_prod = add dot_prod.1 dot_prod.2"
     *
     * Definitions of variables as the result of a macro operation get expanded into the sub-macro operations that make up the macro.
     * Suppose the following macro was defined at the top of the Shape Program:
     * "macro c = my_macro a b; define intvar p; declare p = add a b; define c = mul p p"
     * The line "define z = my_macro x y" becomes the following:
     * (Assume z, x and y have all been declared, and my_macro is being used for the 3rd time (reference counts are 0 indexed)):
     * "declare z_p:2"
     * "define z_p:2 = add x y"
     * "define z = mul z_p:2 z_p:2"
     *
     * Definitions of vectors as operations on other vectors are expanded into component-wise operations.
     * For example, "define_vector z = my_macro x q" gets expanded as follows:
     * (Assume z and x are three-dimensional vectors, q is a scalar variable, and my_macro is the same user-defined binary macro as in the previous example)
     * (This would now be the 4th, 5th and 6th references of my_macro):
     * 
     * "declare z.0_p:3"
     * "declare z.0_p:3 = add x.0 q"
     * "define z.0 = mul z.0_p:3 z.0_p:3"
     * "declare z.1_p:4"
     * "declare z.1_p:4 = add x.1 q"
     * "define z.1 = mul z.1_p:4 z.1_p:4"
     * "declare z.2_p:5"
     * "declare z.2_p:5 = add x.2 q"
     * "define z.2 = mul z.2_p:5 z.2_p:5"
     *
     *
     * Finally, if a line is the definition of a user macro, parse_macro_line is called to parse and store this macro.
     * Nothing is written to the Expanded Program for macro definitions.
     *
     * This method reads from the Program, and writes to an Expanded Program file.
     * The Expanded Shape Program can now be compiled or interpreted.
     *
     * Returns 0 on success, and the appropriate error code otherwise (see utilities.h).
     */
    int expand_program(const string& shape_prog_filename, const string& expanded_shape_prog_filename);

    /* Writes the appropriate expansion of PROG_LINE into EXP_PROG.
     * See the comment for the expand_program for details on what "expansion" entails.
     * If no expansion is needed, then the line is copied directly into EXP_PROG.
     *
     * Returns the number of lines written to exp_prog.
     * This is 1 if no expansion was needed, and the appropriate error code (see utilities.h) if the given line was invalid.
     * Returns 0 if prog_line is an empty line.
     */
    int expand_line(const string& prog_line, ofstream& exp_prog); 



    /* ---------------------------------- Main Expansion Methods ----------------------------------- */



     /* Expands a DECLARE_VECTOR instruction, copying the expanded lines into EXP_PROG.
     * The declare_vector instruction gets broken down into as many regular declare instructions as there are elements in the vector.
     *
     * ex. "declare_vector input x 3" becomes:
     * "declare input x.0"
     * "declare input x.1"
     * "declare input x.2"
     *
     * This method returns the dimension of the vector, which will always be a positive integer.
     */
    int expand_declare_vector_instruction(const string& line, ofstream& exp_prog);

    /* Expands a DEFINE instruction, copying the expanded lines into EXP_PROG.
     * DEFINE instructions need expanding if they involve vector operations or user-defined macros.
     * Dot product operations are expanded into component wise operations (see dot product expansion methods below).
     * Macro instructions are expanded directly based on the user given macro (see expand_unary_macro and expand_binary_macro).
     *
     * If the instruction requires no expanding, nothing is done and 0 is returned.
     * Otherwise, this method returns the number of expanded lines.
     */
    int expand_define_instruction(const string& line, ofstream& exp_prog);

    /* Expands a DEFINE_VECTOR instruction, copying the expanded lines into EXP_PROG.
     * DEFINE_VECTOR instructions define a vector, as opposed to DEFINE instructions which define a scalar variable.
     * This method expands the DEFINE_VECTOR instruction into operations on vector components.
     *
     * Every DEFINE_VECTOR instruction resembles "define_vector <vec_name> = <operand_vector> <optional operand2 (vector, variable or constant)>"
     * A vector can be defined in one of the following ways:
     *
     * 1. as a binary primitive/macro operation on two vectors of the same dimension.
     * 2. as a binary primitive/macro operation on a vector and a scalar variable.
     * 3. as a binary primitive/macro operation on a vector and a constant.
     * 4. as a unary primitive/macro operation on a vector.
     *
     * A DEFINE_VECTOR operation executes the given operation on every component of the vector.
     * For example, "define_vector z = add x y" adds vectors x and y component-wise and the resulting vector is z.
     * It gets expanded to {"define z.0 = add x.0 y.0", "define z.1 = add x.1 y.1", "define z.2 = add x.2 y.2", etc.}.
     *
     * "define_vector z = my_binary_macro x q" defines the i-th component of the vector z as the result of applying my_binary_macro to x[i] and q.
     * It gets expanded into component-wise macro operations, which are further expanded using the rules of macro expansion.
     *
     * Note that DOT PRODUCT operations fall under DEFINE lines, since they define a variable, not a vector.
     *
     * This method returns the number of expanded lines, which is equal to the dimension of the vectors involved.
     */
    int expand_define_vector_instruction(const string& line, ofstream &exp_prog);

    /* This method expands a DEFINE instruction that defines a variable/vector as the result of a vector operation.
	 * RESULT is the variable/vector being defined, and OPERAND1 and OPERAND2 are the operand vectors/constants/variables.
	 *
	 * Based on the given OPER_TYPE, this method dispatches the work to the appropriate expansion method.
	 * See specific vector operation expansion methods below.
	 *
	 * The expanded lines are written into EXP_PROG.
	 * Returns the number of expanded lines.
	 */
	int expand_vector_operation(const OperationType& oper_type, const string& result, const string& operand1, const string& operand2,
    	ofstream& exp_prog);


	/* ------------------------------------ Helper Macro Expansion Methods ----------------------------------- */



    /* Expands a DEFINE instruction that defines a variable as the result of a user-defined unary macro (one operand).
     * Grabs the macro struct with the given MACRO_NAME.
     * Recall this struct was created during the parsing of the macro definitions.
     * Replaces the current instruction with the appropriate sequence of instructions as stored in the macro struct.
     * Replaces the dummy variable names with the given OPERAND and RESULT names.
     * Writes the expanded instructions into EXP_PROG.
     *
     * Returns the number of expanded lines.
     */
    int expand_unary_macro(const string& macro_name, const string& operand, const string& result, 
    	ofstream& exp_prog);

    /* Does the same thing as expand_unary_macro, but for a binary macro.
     */
	int expand_binary_macro(const string& macro_name, const string& operand1, const string& operand2, const string& result, 
    	ofstream& exp_prog);

	/* Replaces all the dummy variable names in DUMMY_LINE with the appropriate argument names RESULT, OPERAND1, and OPERAND2.
	 * For example, "substitute_dummy_names('define r = mul p q', r, p, q, z, x, y)" would return 'define z = mul x y'
	 * This method is used when expanding macros, since the dummy variables from the macro definition must be replaced with the current variables.
	 *
	 * The int MACRO_NUM_REFERENCES is the number of times this macro has been referenced.
	 * This is necessary for macros that involve declaring intvars.
	 * For example, consider the macro:
	 *	"#macro c = my_macro a b; declare intvar foo; define foo = add a b; define c = mul foo 2;"
	 * The first time a variable was defined as the result of a macro, say, "define m = my_macro x y",
	 *	* the intvar foo would be expanded to foo0.
	 * The next time, say, "define n = my_macro p q", the intvar foo would be expanded to foo1.
	 * 
	 * Returns the modified string with the argument names in place of the dummy names.
	 */
	string substitute_dummy_names(const string& dummy_line, string dummy_result, string dummy_op1, string dummy_op2,
    	const string& result, const string& operand1, const string& operand2, int macro_num_references);

	/* Helper methods to determine whether the macro with the given NAME is binary or unary. */
	bool is_binary_macro(const string& name);
	bool is_unary_macro(const string& name);


	
	/* ------------------------------------ Helper Vector Expansion Methods ----------------------------------- */



	/* Expands a DEFINE instruction that defines a variable RESULT as the result of the dot product between VECTOR1 and VECTOR2.
     * The dot product of two vectors get broken up into several multiplications and additions.
     *
     * ex. "define_dot dot_prod = dot a b" becomes the following: (Assume dot_prod, a and b have been declared, and a and b are both two-element vectors.)
     * "declare dot_prod.1"
     * "declare dot_prod.2"
     * "define dot_prod.1 = mul a.1 b.1"
     * "define dot_prod.2 = mul a.2 b.2"
     * "define dot_prod = add dot_prod.1 dot_prod.2"
     *
     * The expanded lines are written into EXP_PROG.
     * Returns the number of expanded lines.
     */
	int expand_dot_product_instruction(const string& result, const string& vector1, const string& vector2, int dimension,
    	ofstream& exp_prog);

	/* Expands a DEFINE instruction that defines a vector RESULT_VEC as the result of component-wise addition between VECTOR1 and VECTOR2.
     * This operation gets broken up into several scalar additions.
     *
     * ex. "define sum_vec = component_wise_add a b" becomes the following: (Assume sum_vec, a and b have been declared, and are all two-element vectors.)
     * "define sum_vec.1 = add a.1 b.1"
     * "define sum_vec.2 = add a.2 b.2"
     *
     * The expanded lines are written into EXP_PROG.
     * Returns the number of expanded lines.
     */
	int expand_component_wise_add_instruction(const string& result_vec, const string& vector1, const string& vector2, int dimension, 
    	ofstream& exp_prog);

	/* Expands a DEFINE instruction that defines a vector RESULT_VEC as the result of component-wise multiplication between VECTOR1 and VECTOR2.
     * This operation gets broken up into several scalar multiplications.
     *
     * ex. "define prod_vec = component_wise_mul a b" becomes the following: (Assume prod_vec, a and b have been declared, and are all two-element vectors.)
     * "define prod_vec.1 = mul a.1 b.1"
     * "define prod_vec.2 = mul a.2 b.2"
     *
     * The expanded lines are written into EXP_PROG.
     * Returns the number of expanded lines.
     */
	int expand_component_wise_mul_instruction(const string& result_vec, const string& vector1, const string& vector2, int dimension, 
    	ofstream& exp_prog);

	/* Expands a DEFINE instruction that defines a vector RESULT_VEC as the result scaling VECTOR by the given SCALING_FACTOR.
	 * The scaling factor could be a variable or a constant.
     * This operation gets broken up into several scalar multiplications.
     *
     * ex. "define scaled_vec = scale_vector a s" becomes the following:
     * "define scaled_vec.1 = mul a.1 s"
     * "define scaled_vec.2 = mul a.2 s"
	 *
     * (Assume scaled_vec and a have been declared as two-element vectors, and s has been declared and defined.)
     *
     * The expanded lines are written into EXP_PROG.
     * Returns the number of expanded lines.
     */
	int expand_scale_vector_instruction(const string& result_vec, const string& vector1, const string& scaling_factor, int dimension,
		ofstream& exp_prog);

	/* Expands a DEFINE instruction that defines a vector RESULT_VEC as the result incrementing each component in VECTOR by the given INCREMENTING_FACTOR.
	 * The incrementing factor could be a variable or a constant.
     * This operation gets broken up into several scalar additions.
     *
     * ex. "define incr_vec = increment_vector a s" becomes the following:
     * "define incr_vec.1 = add a.1 s"
     * "define incr_vec.2 = add a.2 s"
	 *
     * (Assume incr_vec and a have been declared as two-element vectors, and s has been declared and defined.)
     *
     * The expanded lines are written into EXP_PROG.
     * Returns the number of expanded lines.
     */
	int expand_increment_vector_instruction(const string& result_vec, const string& vector1, const string& incrementing_factor, int dimension, 
    	ofstream& exp_prog);



    /* ------------------------------- Macro Parsing Methods ----------------------------- */



	/* Parses the given MACRO_LINE, and populates the given MACRO struct.
	 * Calls parse_macro_first_line to obtain the macro name, result name, and dummy operand names for the macro.
	 * For each subsequent line of the macro definition, parse_macro_subsequent_line is called.
	 * These calls populate the "lines" field of the macro struct.
	 *
	 * The goal of parsing macro definitions is so that later in the program,
	 * 	when a variable is defined as the result of a macro operation,
	 *  that definition line can easily be replaced with the lines of the macro (see expand_unary_macro, expand_binary_macro).
	 *
	 * Returns 0 on success, or an error code if the macro definition is invalid (see utilities.h).
	 */
	int parse_macro_line(const string& macro_line, struct macro *macro);

	/* Parses the first line a macro.
	 * Stores the name of the macro, the dummy result name, and the dummy operand names in the macro struct.
	 * For example, "#macro z = my_macro x y" would store in the MACRO struct:
	 *	name: my_macro, result: z, operand1: x, operand2: y, is_binary: true
	 * 
	 * Returns 0 on success, or an error code if the line is invalid.
	 */
	int parse_macro_first_line(const string& first_line, struct macro *macro);

	/* Parses a subsequent (not the first) line of a macro.
	 * If the line is valid, adds it to the vector of lines in the MACRO struct.
	 *
	 * Returns 0 on success, or an error code if the line is invalid.
	 */
	int parse_macro_subsequent_line(const string& line, struct macro *macro);

	



    /* ------------------------------ Error Checking Methods ---------------------------- */


    /* Determines whether the given LINE is a valid DECLARE instruction.
     * In order to be valid, there must be exactly 3 tokens.
     * The first must be the word "declare", the second a valid variable type, and the third a valid variable name.
     * See the invalid_var_name function in utilities.h for what constitutes a valid variable name.
     * This must be the first declaration of this variable name.
     *
     * Returns 0 if the instruction is valid, or an error code otherwise (see utilities.h).
     */
    int is_valid_declare_line(const string& line);

    /* Determines whether the given LINE is a valid DEFINE instruction.
     * In order to be valid, there must be exactly 4 tokens.
     * The first must be the word "declare_vector" and the second a valid variable type.
     * The third must be a valid variable name and the fourth an int (the dimension of the vector).
     * The size of the vector must be a positive integer and cannot exceed MAX_VECTOR_SIZE (defined in utilities.h).
     * This must be the first declaration of this variable name.
     * See the invalid_var_name function in utilities.h for what constitutes a valid variable name.
     *
     * Returns 0 if the instruction is valid, or an error code otherwise (see utilities.h).
     */
    int is_valid_declare_vector_line(const string& line);

    /* Determines whether the given LINE is a valid DEFINE instruction.
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
     * 3. "define <var_name> = <dot product> <vector operand 1> <vector operand 2>"
     * 	- for dot product operations, both operand vectors must be of the same dimension.
     *
     * 4. "define <var_name> = <constant>"
     * 
     * 5. "define <var_name> = <another variable with a different name>"
     *
     * This method returns 0 if the instruction is valid, or an error code otherwise (see utilities.h)
     */
    int is_valid_define_line(const string& line);

    /* Determines whether the given LINE is a valid DEFINE_VECTOR instruction.
     * A vector must be declared before it is defined, and cannot be defined twice.
     * All the operand vectors/variables must be defined.
     * The dimension of the defined vector must match the dimensions of all operands that are vectors.
     * Input, weight, and expected output vectors cannot be defined.
     *
     * Every DEFINE_VECTOR instruction resembles "define_vector <vec_name> = <operand_vector> <optional operand2 (vector, variable or constant)>"
     * A vector can be defined in one of the following ways:
     *
     * 1. as a binary primitive/macro operation on two vectors of the same dimension.
     * 2. as a binary primitive/macro operation on a vector and a scalar variable.
     * 3. as a binary primitive/macro operation on a vector and a constant.
     * 4. as a unary primitive/macro operation on a vector.
     *
     * A DEFINE_VECTOR operation executes the given operation on every component of the vector.
     * For example, "define_vector z = add x y" adds vectors x and y component-wise and the resulting vector is z.
     * "define_vector z = my_binary_macro x q" defines the i-th component of the vector z as the result of applying my_binary_macro to x[i] and q.
     *
     * Note that DOT PRODUCT operations are DEFINE instructions, since they define a scalar variable, not a vector
     *
     * This method returns 0 if the instruction is valid, or an error code otherwise (see utilities.h)
     */
    int is_valid_define_vector_line(const string& line);


    /* Returns whether a macro with the given NAME has been successfully defined earlier. */
	bool is_valid_macro(const string& name);

	
};



#endif
