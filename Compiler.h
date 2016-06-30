#ifndef COMPILER_H
#define COMPILER_H

#include "Node.h"
#include "DataFlowGraph.h"
#include "utilities.h"

using namespace std;


/* The purpose of compilation is to translate the Shape Program into the Gradient Computing Program (GCP).
 * Compilation occurs in these three steps:
 *
 *  1. Parse the Shape Program line by line, building up the Data Flow Graph.
 *      Copy each line directly into the GCP.
 *  2. Topologically sort the Data Flow Graph.
 *  3. Visit the nodes in the sorted order.
 *      At each node, add lines to the GCP that declare and define the appropriate partial derivative variables.
 */
 
class Compiler {

    /* This Data Flow Graph is built up over the course of compilation.
     * Each variable in the Shape Program is represented by a node.
     * The directed edges represent dependencies.
     * A topological sort gives the order in which nodes are to be visited when computing partial derivatives.
     * This ordering determines the layout of the GCP (the output of the Compile Phase).
     */
    DataFlowGraph *dfg;

public:

    /* Constructor.
     * Initializes a new Data Flow Graph.
     */
    Compiler();

    /* This method builds the GCP, through the following steps:
     * 
     * Iterates through all the lines of the Shape Program, sending each line to be parsed, and copying it into the GCP.
     * After the parsing phase, the Data Flow Graph is complete.
     * Topologically sorts the DFG.
     * Visits each node in order, copying the declarations and definitions of partial derivative variables into the GCP.
     * 
     * Returns 0 on success, and the appropriate error code otherwise (see utilities.h).
     */
    int compile(const string& shape_prog_filename, const string& gcp_filename);

    /* Reads one line of code, and takes the appropriate actions.
     * If the line is the declaration of a variable, a node is added to the Data Flow Graph.
     * If the line defines an expression for the variable, the respective node is updated.
     * Its operation is set, and the appropriate edges are added between nodes.
     * Once this method is called on every line, the Data Flow Graph is ready for the next step, the Topological sort.
     *
     * This method returns 0 if the line was successfully parsed.
     * Returns the appropriate error code otherwise (see utilities.h).
     */
    int parse_line(char line[]);

    /* Populates gcp_line with a near duplicate of shape_line.
     * The variable type may be changed, though.
     * Inputs, weights and expected outputs from the Shape Program all become inputs in the GCP.
     * Outputs, intvars, and loss variables from the Shape Program all become intvars in the GCP.
     * Returns DUPLICATE_SUCCESS_DECLARE or DUPLICATE_SUCCESS_DEFINE on success (see utilities.h).
     * Returns the appropriate error code (see utilities.h) on failure.
     */
    int duplicate_line_for_gcp(char shape_line[], char gcp_line[]);

};



/* ------------------------ Helper Functions ------------------- */

/* Returns a string that is the name of the partial derivative of var 1 with respect to var 2.
 * If var1 were "foo", and var2 were "bar", this method would return "d/foo/d/bar".
 */
string generate_partial_var_name(const string& var1, const string& var2);

/* Adds the declaration of a partial derivative to the GCP.
 * The variable is the partial derivative of the Loss variable with respect to the variable represented by the given node.
 * Returns the name of this variable.
 */
string declare_partial_lambda(Node *node, string loss_name, ofstream &gcp);

/* Adds the definition of a partial derivative to the GCP.
 * The variable defined is the partial derivative of the Loss variable with respect to the variable represented by the given node.
 * partial(Loss, x) = partial(Loss, x.parent) * partial(x.parent, x)
 */ 
void define_partial_lambda(Node *node, string loss_name, ofstream &gcp, string partial_var_name);

/* These two methods are nearly identical.
 * They add the declaration of a partial derivative to the GCP.
 * This is the partial derivative of the given node with respect to its first/second child.
 * Returns the name of this variable.
 * If the given node doesn't have a first/second child (or its first/second child is constant), returns an empty string.
 */
string declare_child_one_partial(Node *node, ofstream &gcp);
string declare_child_two_partial(Node *node, ofstream &gcp);

/* These two methods are nearly identical.
 * They add the definition of a partial derivative to the GCP.
 * This is the partial derivative of the given node with respect to its first/second child.
 * This partial derivative is calculated using basic Calculus rules for partial differentiation.
 */
void define_child_one_partial(Node *node, ofstream &gcp, string child_one_partial);
void define_child_two_partial(Node *node, ofstream &gcp, string child_two_partial);




#endif
