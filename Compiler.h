#ifndef TENSORFLOW_H
#define TENSORFLOW_H

#include "Node.h"
#include "DataFlowGraph.h"

using namespace std;


enum instruction_type_t {
    DECLARE,
    DEFINE,
    INVALID_INST
};

enum variable_type_t {
    INPUT,
    OUTPUT,
    EXP_OUTPUT,
    WEIGHT,
    INTVAR,
    LOSS,
    CONSTANT,
    INVALID_VAR_TYPE
};

enum operation_type_t {
    ADD,
    MUL,
    INVALID_OPERATION
};

enum instruction_type_t get_instruction_type(char inst[]);
enum variable_type_t get_variable_type(char var_name[]);
enum operation_type_t get_operation_type(char oper[]);


class Compiler {
    DataFlowGraph *dfg;
public:
    Compiler();
    void compile(char *shape_prog_filename, DataFlowGraph *dfg, char *gcp_filename);
    int parse_line(char line[]);
    void declare_partial_lambda(Node *node, char *loss_name, ofstream &gcp);
    void define_partial_lambda(Node *node, char *loss_name, ofstream &gcp, char partial_var_name[]);

};



#endif
