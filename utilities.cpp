#include "utilities.h"
#include <fstream>

using namespace std;

InstructionType get_instruction_type(const string &inst_type) {
    if (inst_type.compare("declare") == 0) {
        return InstructionType::DECLARE;
    }
    if (inst_type.compare("define") == 0) {
        return InstructionType::DEFINE;
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

    return OperationType::INVALID_OPERATION;
}


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


bool invalid_file_name(const string& filename) {
    ifstream f(filename);
    bool valid = f.good();

    if (f.is_open()) {
        f.close();
    }
    return !valid;
}


bool invalid_var_name(const string& name) {
    if (name.compare("") == 0) {
        return true;
    }
    return false;
}
