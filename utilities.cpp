#include "utilities.h"
#include <fstream>


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
    if (oper.compare("deriv_logistic") == 0) {
        return OperationType::DERIV_LOGISTIC;
    }
    if (oper.compare("reciprocal") == 0) {
        return OperationType::RECIPROCAL;
    }
    if (oper.compare("pow") == 0) {
        return OperationType::POW;
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


bool invalid_file_name(const string& filename) {
    ifstream f(filename);
    bool valid = f.good();

    if (f.is_open()) {
        f.close();
    }
    return !valid;
}


bool is_valid_var_name(const string& name) {
    if (name.compare("") == 0) return false;
    if (name.find_first_of("1234567890") != string::npos) return false;
    if (get_instruction_type(name) != InstructionType::INVALID_INST) return false;
    if (get_variable_type(name) != VariableType::INVALID_VAR_TYPE) return false;
    if (get_operation_type(name) != OperationType::INVALID_OPERATION) return false;

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
        oper_type == OperationType::DERIV_LOGISTIC || oper_type == OperationType::RECIPROCAL || oper_type == OperationType::POW;
}

bool is_valid_vector_operation(const string& name) {
    OperationType oper_type = get_operation_type(name);
    return oper_type == OperationType::DOT || oper_type == OperationType::SCALE_VECTOR || oper_type == OperationType::INCREMENT_VECTOR;
}

bool is_binary_primitive(const string& name) {
    OperationType oper_type = get_operation_type(name);
    return oper_type == OperationType::ADD || oper_type == OperationType::MUL || oper_type == OperationType::POW;
}

bool is_unary_primitive(const string& name) {
    OperationType oper_type = get_operation_type(name);
    return oper_type == OperationType::LOGISTIC || oper_type == OperationType::DERIV_LOGISTIC || oper_type == OperationType::RECIPROCAL;
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
    return get_instruction_type(name) == InstructionType::INVALID_INST;
}

