#include <iostream>
#include <stdlib.h>

#include "Interpreter.h"
#include "Preprocessor.h"

using namespace std;


void interpreter_exit_with_usage() {
    cerr << "\nMust provide the name of a TenFlang file, and the name of the file from which the inputs are read." << endl;
    cerr << "If your program has already been pre-processed, these two arguments suffice." << endl;
    cerr << "Example: " << endl;
    cerr << "# ./interpreter my_program.tf inputs.txt" << endl << endl;
    cerr << "If your program has not yet been pre-processed, use the '-pp' flag and specify the name of the file to which you want the Expanded Program written." << endl;
    cerr << "Example: " << endl;
    cerr << "# ./compiler my_program.tf inputs.txt -pp temp_expanded_program.tf" << endl << endl;
    exit(EXIT_FAILURE);
}
 


int main(int argc, char *argv[]) {

	if (argc != 3 && argc != 5) {
        interpreter_exit_with_usage();
    }

    // determine whether the given program has already been preprocessed
    bool already_preprocessed = true;
    string exp_prog = "";
    if (argc == 5) {
        if (string(argv[3]) != "-pp") {
            interpreter_exit_with_usage();
        }
        exp_prog = string(argv[4]);
        already_preprocessed = false;
    }

    Interpreter i;
    int interpret_success;
    string prog(argv[1]);
    string inputs(argv[2]);

    // if the given program (2nd command line token) is not already preprocessed, preprocess it
    // store the temp expanded program into the file whose name is given by the 5th command line token
    // intepret the expanded program, reading inputs from the file given by the 3rd command line token
    if (!already_preprocessed) {
        Preprocessor p;
        string exp_prog(argv[4]);
        int preprocess_success = p.expand_program(prog, exp_prog);
        if (preprocess_success != 0) {
            return preprocess_success;
        }

        interpret_success = i.interpret_program(exp_prog, inputs);
        return interpret_success;
    }
    // if the given program (2nd command line token) is a preprocessed (expanded) program, simply compile it
    // intepret the given program, reading inputs from the file given by the 3rd command line token
    else {
        interpret_success = i.interpret_program(prog, inputs);
        return interpret_success;

    }

}
