#include "utilities.h"
#include <fstream>
#include <iostream>


using namespace std;



/* -------------- Enum Classes ---------------- */

InstructionType get_instruction_type(const string &inst_type) {
    if (inst_type.compare("declare") == 0) {
        return InstructionType::DECLARE;
    }
    if (inst_type.compare("define") == 0) {
        return InstructionType::DEFINE;
    } 
    if (inst_type.compare("declare_vector") == 0) {
        return InstructionType::DECLARE_VECTOR;
    }
    if (inst_type.compare("#macro") == 0) {
        return InstructionType::MACRO;
    }
    
    return InstructionType::INVALID_INST;
}

VariableType get_variable_type(const string &var_type) {
    if (var_type.compare("input") == 0) {
        return VariableType::INPUT;
    }
    if (var_type.compare("output") == 0) {
        return VariableType::OUTPUT;
    }
    if (var_type.compare("exp_output") == 0) {
        return VariableType::EXP_OUTPUT;
    }
    if (var_type.compare("weight") == 0) {
        return VariableType::WEIGHT;
    }
    if (var_type.compare("intvar") == 0) {
        return VariableType::INTVAR;
    }
    if (var_type.compare("loss") == 0) {
        return VariableType::LOSS;
    }
    if (var_type.compare("constant") == 0) {
        return VariableType::CONSTANT;
    }
   
    return VariableType::INVALID_VAR_TYPE; 
}

OperationType get_operation_type(const string &oper) {
    if (oper.compare("add") == 0) {
        return OperationType::ADD;
    }
    if (oper.compare("mul") == 0) {
        return OperationType::MUL;
    }
    if (oper.compare("dot") == 0) {
        return OperationType::DOT;
    }
    if (oper.compare("logistic") == 0) {
        return OperationType::LOGISTIC;
    } 
    if (oper.compare("exp") == 0) {
        return OperationType::EXP;
    }
    if (oper.compare("pow") == 0) {
        return OperationType::POW;
    }
    if (oper.compare("ln") == 0) {
        return OperationType::LN;
    }
    if (oper.compare("scale_vector") == 0) {
        return OperationType::SCALE_VECTOR;
    }
    if (oper.compare("increment_vector") == 0) {
        return OperationType::INCREMENT_VECTOR;
    } 
    if (oper.compare("component_wise_add") == 0) {
        return OperationType::COMPONENT_WISE_ADD;
    }
    if (oper.compare("component_wise_mul") == 0) {
        return OperationType::COMPONENT_WISE_MUL;
    }

    return OperationType::INVALID_OPERATION;
}



/* ------------------ Helper Methods ---------------- */

bool is_constant(const string& name) {
    size_t size;
    float f;

    try {
        f = stof(name, &size);
    } catch(const exception& e) {
        return false;
    }

    if (size == name.length()) {
        return true;
    }

    return false;
}

bool is_int(const string& name) {
    size_t size;
    int i;

    try {
        i = stoi(name, &size);
    } catch(const exception& e) {
        return false;
    }

    if (size == name.length()) {
        return true;
    }

    return false;
}


bool is_valid_file_name(const string& filename) {
    ifstream f(filename);
    bool valid = f.good();

    if (f.is_open()) {
        f.close();
    }
    return valid;
}


bool is_valid_var_name(const string& name) {
    if (name.compare("") == 0) return false;
    if (name.find_first_of("1234567890") != string::npos) return false;
    if (is_keyword(name)) return false;
    return true;
}

bool is_valid_operation(const string& oper_name) {
    return get_operation_type(oper_name) != OperationType::INVALID_OPERATION;
}

bool is_valid_instruction(const string& inst_name) {
    return get_instruction_type(inst_name) != InstructionType::INVALID_INST;
}

bool is_valid_primitive(const string& name) {
    OperationType oper_type = get_operation_type(name);
    return oper_type == OperationType::ADD || oper_type == OperationType::MUL || oper_type == OperationType::LOGISTIC || 
        oper_type == OperationType::EXP || oper_type == OperationType::POW || oper_type == OperationType::LN;
}

bool is_valid_vector_operation(const string& name) {
    OperationType oper_type = get_operation_type(name);
    return oper_type == OperationType::DOT || oper_type == OperationType::SCALE_VECTOR || oper_type == OperationType::INCREMENT_VECTOR
        || oper_type == OperationType::COMPONENT_WISE_ADD || oper_type == OperationType::COMPONENT_WISE_MUL;
}

bool is_binary_primitive(const string& name) {
    OperationType oper_type = get_operation_type(name);
    return oper_type == OperationType::ADD || oper_type == OperationType::MUL || oper_type == OperationType::POW;
}

bool is_unary_primitive(const string& name) {
    OperationType oper_type = get_operation_type(name);
    return oper_type == OperationType::LOGISTIC || oper_type == OperationType::EXP || oper_type == OperationType::LN;
}

bool is_binary_vector_operation(const string& name) {
    OperationType oper_type = get_operation_type(name);
    return oper_type == OperationType::DOT || oper_type == OperationType::COMPONENT_WISE_ADD || oper_type == OperationType::COMPONENT_WISE_MUL;
}

bool is_unary_vector_operation(const string& name) {
    OperationType oper_type = get_operation_type(name);
    return oper_type == OperationType::SCALE_VECTOR || oper_type == OperationType::INCREMENT_VECTOR;
}


bool is_valid_vector_size(int size) {
    return size > 0 && size <= MAX_VECTOR_SIZE;
}


bool is_valid_macro_name(const string& name) {
    return !is_keyword(name);
}


bool is_keyword(const string& word) {
    if (get_instruction_type(word) != InstructionType::INVALID_INST) return true;
    if (get_variable_type(word) != VariableType::INVALID_VAR_TYPE) return true;
    if (get_operation_type(word) != OperationType::INVALID_OPERATION) return true;
    if (word.compare("=") == 0) return true;
    return false;
}



/* ------------------------------ Tokenizer Method ----------------------------- */


int tokenize_line(const string& line, vector<string> *tokens, const string& delimiters) {

    if (tokens == NULL) return OTHER_ERROR;
    if (line.compare("") == 0) return 0;

    // remove spaces from the beginning
    size_t first_non_delimiter = line.find_first_not_of(delimiters, 0);
    if (first_non_delimiter == string::npos) return 0;

    string modified_line;
    if (first_non_delimiter > 0) {
        modified_line = line.substr(first_non_delimiter, string::npos);
    } else {
        modified_line = line;
    }

    // now we can be sure modified_line does not have leading delimiters
    size_t curr_delim_pos = 0;
    size_t next_delim_pos = 0;
    size_t num_tokens = 0;
    
    while (curr_delim_pos != string::npos) {
        
        // start looking for the next delimiter, starting at the position after the most recently found delimiter
        if (curr_delim_pos == 0) {
            next_delim_pos = modified_line.find_first_of(delimiters, 0);
        } else {
            next_delim_pos = modified_line.find_first_of(delimiters, curr_delim_pos + 1);
        }
        
        // if there are multiple delimiters in a row, we keep moving on
        if (next_delim_pos > curr_delim_pos + 1) {
            // make sure we don't substring out of bounds
            if (curr_delim_pos + 1 < modified_line.length()) {
                if (curr_delim_pos == 0) {
                    tokens->push_back(modified_line.substr(0, next_delim_pos));
                } else {
                    tokens->push_back(modified_line.substr(curr_delim_pos + 1, next_delim_pos - (curr_delim_pos + 1)));
                }
                num_tokens++;
            }
        }
        
        curr_delim_pos = next_delim_pos;
    }

    return num_tokens;

}

