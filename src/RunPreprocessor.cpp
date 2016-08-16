#include <stdlib.h>
#include <iostream>

#include "Preprocessor.h"

using namespace std;


void preprocessor_exit_with_usage() {
	cerr << "\nMust provide the name of a TenFlang file, and the name of the file to which the Expanded Program will be written." << endl;
	cerr << "Example: " << endl;
	cerr << "# ./preprocessor my_shape_program.tf my_expanded_shape_program.tf" << endl << endl;
	exit(EXIT_FAILURE);
}


/* Runs the Preprocessor.
 * The first argument is the name of the file from which the user-given Program is read.
 * The second argument is the name of the file to which the Expanded Program is written.
 */

int main(int argc, char *argv[]) {

	if (argc != 3) {
		preprocessor_exit_with_usage();
	}

	Preprocessor p;
	string prog(argv[1]), exp_prog(argv[2]);
	int preprocess_success = p.expand_program(prog, exp_prog);
	return preprocess_success;

}