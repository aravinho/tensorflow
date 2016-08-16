#include <iostream>
#include <stdlib.h>

#include "Compiler.h"
#include "Preprocessor.h"

using namespace std;


void compiler_exit_with_usage() {
    cerr << "\nMust provide the name of a TenFlang file, and the name of the file to which the GCP will be written." << endl;
    cerr << "If your program has already been pre-processed, these two arguments suffice." << endl;
    cerr << "Example: " << endl;
    cerr << "# ./compiler my_expanded_shape_program.tf my_gcp.tf" << endl << endl;
    cerr << "If your program has not yet been pre-processed, use the '-pp' flag and specify the name of the file to which you want the Expanded Program written." << endl;
    cerr << "Example: " << endl;
    cerr << "# ./compiler my_shape_program.tf my_gcp.tf -pp temp_expanded_shape_program.tf" << endl << endl;
    exit(EXIT_FAILURE);
}
 
/* Runs the Compiler.
 * The first argument is the name of the file from which the Shape Program is read.
 * The second argument is the name of the file to which the GCP is written.
 * If the Shape Program needs to be pre-processed, the third argument must be "-pp",
 *  and the fourth argument is the name of the file to which the Expanded Shape Program is written.
 */
int main(int argc, char *argv[]) {

    if (argc != 3 && argc != 5) {
        compiler_exit_with_usage();
    }

    // determine whether the given program has already been preprocessed
    bool already_preprocessed = true;
    string exp_prog = "";
    if (argc == 5) {
        if (string(argv[3]) != "-pp") {
            compiler_exit_with_usage();
        }
        exp_prog = string(argv[4]);
        already_preprocessed = false;
    }


    Compiler c;
    int compile_success;
    string shape_prog(argv[1]);
    string gcp(argv[2]);

    // if the given program (2nd command line token) is not already preprocessed, preprocess it
    // store the temp expanded program into the file whose name is given by the 5th command line token
    // compile the expanded program into the GCP, writing the GCP into the file given by the 3rd command line token
    if (!already_preprocessed) {
        Preprocessor p;
        string exp_shape_prog(argv[4]);
        int preprocess_success = p.expand_program(shape_prog, exp_shape_prog);
        if (preprocess_success != 0) {
            return preprocess_success;
        }

        compile_success = c.compile(exp_shape_prog, gcp);
        return compile_success;
    }
    // if the given program (2nd command line token) is a preprocessed (expanded) program, simply compile it
    // write the GCP into the file whose name is given by the 3rd command line token
    else {
        compile_success = c.compile(shape_prog, gcp);
        return compile_success;

    }

}
