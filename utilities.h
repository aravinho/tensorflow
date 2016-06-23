#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

using namespace std;


/* Defines enum classes and associated getter methods.
 * These enum classes represent Instruction Types, Variable Types, or Operation Types.
 * Also defines various assorted helper methods.
 */

/* --------------------- Enum Classes and Methods ------------------- */

/* An instruction in a Shape Program is either a Declaration or a Definition.
 */
enum class InstructionType {
    DECLARE,
    DEFINE,
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

/* There are two operations, addition and multiplication.
 * More differentiable operations will be added.
 */
enum class OperationType {
    ADD,
    MUL,
    INVALID_OPERATION
};


/* These methods return the type that corresponds to the value of the given string.
 */
InstructionType get_instruction_type(const std::string &inst_type);
VariableType get_variable_type(const string &var_type);
OperationType get_operation_type(const string &oper);



/* ------------------------- Assorted Helper Methods ------------------- */

/* Returns true if the given string cannot be parsed as a float.
 * The try-catch block is because the stof method throws an exception if the string cannot be parsed as a float.
 */
bool is_constant(const string& name);

/* Returns true if there is no file with the given filename.
 */
bool invalid_file_name(const string& filename);

/* Returns true if the given name is an invalid variable name.
 * Empty strings are invalid names.
 * Strings with numerals are invalid names (implement this).
 */
bool invalid_var_name(const string& name);


#endif