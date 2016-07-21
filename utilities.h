#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <vector>

using namespace std;

/* Constants used by the Compiler and Interpreter.
 */
#define MAX_NUM_VARIABLES 100
#define MAX_LINE_LENGTH 1000
#define MAX_SHAPE_PROG_VAR_NAME_LENGTH 80
#define MAX_GCP_VAR_NAME_LENGTH 200
#define MAX_EXPANSION_FACTOR 1000
#define MAX_NUM_TOKENS 100
#define MAX_VECTOR_SIZE 1000

#define INVALID_LINE -1
#define INVALID_VAR_NAME -2
#define VAR_DEFINED_BEFORE_DECLARED -3
#define VAR_DEFINED_TWICE -4
#define VAR_DECLARED_TWICE -5
#define INPUT_VALUE_NOT_PROVIDED -6
#define VAR_REFERENCED_BEFORE_DEFINED -7
#define CANNOT_DEFINE_I_W_EO -8
#define OTHER_ERROR -9
#define BAD_VAR_TYPE -10
#define INVALID_FILE_NAME -11
#define VECTORS_OF_DIFFERENT_DIMENSION -12
#define BAD_VECTOR_SIZE -13
#define INVALID_MACRO_NAME -14
#define MACROS_NOT_AT_TOP -15

#define DUPLICATE_SUCCESS_DECLARE 20
#define DUPLICATE_SUCCESS_DEFINE 21
#define DUPLICATE_SUCCESS_EMPTY_LINE 22


/* Defines enum classes and associated getter methods.
 * These enum classes represent Instruction Types, Variable Types, or Operation Types.
 * Also defines various assorted helper methods.
 */

/* --------------------- Enum Classes and Methods ------------------- */

/* An instruction in a Shape Program is either a Declaration or a Definition.
 */
enum class InstructionType {
    DECLARE,
    DECLARE_VECTOR,
    DEFINE,
    MACRO,
    INVALID_INST
};

/* There are six types of variables, and constants (floats). 
 */
enum class VariableType {
    INPUT,
    OUTPUT,
    EXP_OUTPUT,
    WEIGHT,
    INTVAR,
    LOSS,
    CONSTANT,
    INVALID_VAR_TYPE
};

/* These are differentiable unary or binary operations.
 * More differentiable operations will be added.
 */
enum class OperationType {
    ADD,
    MUL,
    DOT,
    LOGISTIC,
    EXP,
    POW,
    LN,
    SCALE_VECTOR,
    INCREMENT_VECTOR,
    COMPONENT_WISE_ADD,
    COMPONENT_WISE_MUL,
    INVALID_OPERATION
};

/* These operations are used to calculate derivatives of the operations above.
 * DERIV_LOGISTIC and DERIV_POW are not allowed in a user program.
 * These operations are only allowed in a GCP.
 */
enum class DerivativeOperationType {
    ADD,
    MUL,
    DERIV_LOGISTIC,
    EXP,
    DERIV_POW,
    LN,
    INVALID_DERIV_OPERATION
};


/* These methods return the type that corresponds to the value of the given string.
 */
InstructionType get_instruction_type(const std::string &inst_type);
VariableType get_variable_type(const string &var_type);
OperationType get_operation_type(const string &oper);
DerivativeOperationType get_derivative_operation_type(const string& oper);



/* ------------------------- Assorted Helper Methods ------------------- */

/* Returns true if the given string can be parsed as a float.
 * The try-catch block is because the stof method throws an exception if the string cannot be parsed as a float.
 */
bool is_constant(const string& name);

/* Returns true if the given string can be parsed as an int.
 * This is used in declare_vector instructions by the first pass of the compiler.
 */
bool is_int(const string& name);

/* Returns true if there is no file with the given filename.
 */
bool invalid_file_name(const string& filename);

/* Returns true if the given name is a valid variable name, and false otherwise.
 * Empty strings are invalid names.
 * Strings with numerals are invalid names.
 * Names that match a valid instruction, variable or operation type are invalid.
 * Names of macros??
 */
bool is_valid_var_name(const string& name);

/* Returns true if the given name is a valid operation and false otherwise. */
bool is_valid_operation(const string& oper_name);

/* Returns true if the given name is a valid instruction type and false otherwise. */
bool is_valid_instruction(const string& inst_name);

/* Returns true if the given name is a valid derivative operation, and false otherwise. */
bool is_valid_deriv_operation(const string& oper_name);

/* Returns true if the given name is a valid primitive instruction, and false otherwise. */
bool is_valid_primitive(const string& name);

/* Returns true if the given name is a valid vector operation, and false otherwise. */
bool is_valid_vector_operation(const string& name);

/* Returns true if the given name is a binary primitive, and false otherwise. */
bool is_binary_primitive(const string& name);

/* Returns true if the given name is a unary primitive, and false otherwise. */
bool is_unary_primitive(const string& name);

/* Returns true if the given name is a binary derivative operation, and false otherwise. */
bool is_binary_deriv_operation(const string& name);

/* Returns true if the given name is a unary derivative operation, and false otherwise. */
bool is_unary_deriv_operation(const string& name);

/* Returns true if the given name is a binary vector operation, and false otherwise. */
bool is_binary_vector_operation(const string& name);

/* Returns true if the given name is a unary vector operation, and false otherwise. */
bool is_unary_vector_operation(const string& name);

/* Returns true if the given size is a positive integer less than or equal to MAX_VECTOR_SIZE.
 * Returns false otherwise.
 */
bool is_valid_vector_size(int size);

/* Returns true if the given name is a valid macro name, and false otherwise.
 * Macro names can be anything other than the names of primitive/vector operations.
 */
bool is_valid_macro_name(const string& name);

/* Returns true if the given word is a keyword.
 * A keyword is the name of an instruction, the name of an operation,
 *  the name of a variable type, or the equals sign.
 */
bool is_keyword(const string& word);


/* ------------------------ Tokenizer Method -------------------- */

/* Populates the tokens vector with the tokens of LINE.
 * Tokens are delimited by any character in the given DELIMITERS string.
 *
 * Returns the number of tokens, or an error code on failure (see utilities.h).
 */
int tokenize_line(const string& line, vector<string> *tokens, const string& delimiters);


#endif