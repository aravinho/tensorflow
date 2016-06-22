#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include "Compiler.h"

/* Returns true if there is no file with the given FILENAME.
 */
bool invalid_file_name(const char *filename) {
    if (filename == NULL) {
        return true;
    }

    ifstream f(filename);
    bool valid = f.good();

    if (f.is_open()) {
        f.close();
    }
    return !valid;
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Must provide two arguments, a Shape Program file name and a GCP file name." << endl;
        return -1;
    }

    if (invalid_file_name(argv[1])) {
        cerr << "Invalid Shape Program file name." << endl;
        return -1;
    }

    cout << "success" << endl;

    Compiler c;
    //c.compile(argv[1], argv[2]);
    
    /*DataFlowGraph *dfg = new DataFlowGraph();
    compile("shape_prog.tf", dfg, "gcp.tf");
    cout << endl;
    cout << endl;
    cout << "num_nodes: " << dfg->get_num_nodes() << endl;

    unordered_map<string, Node*>* all_nodes = dfg->get_all_nodes();
    Node *node;
    for ( auto it = all_nodes->begin(); it != all_nodes->end(); ++it ) {
        cout << "name: " << it->first << endl;
        node = it->second;
        cout << "node name: " << node->get_name() << ", parent name: " << node->get_parent_name();
        cout << ", child one: " << node->get_child_one_name() << ", child two: " << node->get_child_two_name() << endl;
    }*/
    return 0;
}
