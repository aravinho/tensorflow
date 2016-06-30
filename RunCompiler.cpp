#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include "Compiler.h"


/* Runs the Compiler.
 * The first argument is the name of the file from which the Shape Program is read.
 * The second argument is the name of the file to which the GCP is written.
 */

 
int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Must provide two arguments, a Shape Program file name and a GCP file name." << endl;
        return -1;
    }

    Compiler c;
    c.compile(string(argv[1]), string(argv[2]));
    
    
    return 0;
}
