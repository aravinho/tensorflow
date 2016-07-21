#ifndef GRADIENT_DESCENT_H
#define GRADIENT_DESCENT_H

#include <string>
#include <unordered_map>
#include <vector>

#include "utilities.h"
#include "Interpreter.h"
#include "BindingsDictionary.h"

using namespace std;


/* This file defines functions used in the Weight Calculation Phase.
 * Recall that the purpose of the Weight Calculation Phase is to determine the set of weights
 *	that minimizes the value of the loss variable defined in a Shape Program.
 *
 * The main function in this phase is "calculate_weights".
 * Given a GCP and a set of training data, it returns the loss-minimizing set of values for the weight variables.
 * 
 * See the comments for the individual functions for more detail on the Gradient Descent process.
 */


/* --------------------------------- Some Important Constants ---------------------------------- */


/* The learning rate determines how much the weights are incremented after each iteration of the algorithm. */
#define LEARNING_RATE 0.05
/* This defines how many iterations to stop after. More iterations tends to result in more precision. */
#define MAX_NUM_ITERATIONS 1000
/* This variable determines how the maximum magnitude of the gradient vector after which the algorithm can terminate.
 * If the gradient never becomes this small, the algorithm will terminate after MAX_NUM_ITERATIONS.
 */
#define GRADIENT_PRECISION 0.5


/* A VariableVector is an abstraction used to represent a vector of inputs, outputs or weights.
 * It is a map between variable names and their values.
 */
typedef unordered_map<string, float> VariableVector;


/* --------------------------------- Main Functions --------------------------------------- */

/* This method runs the Gradient Descent Algorithm.
 * It takes in the name of a GCP and a set of training data.
 * The training data is provided as a list of pairs of Variable Vectors.
 * Each pair is an {input VariableVector, output VariableVector} pair.
 *
 * This method also takes in a list of names of weights and partials,
 *	so it knows the names of the weights it needs to increment,
 *	and which partials form the gradient.
 *
 * The Gradient Descent Algorithm works as follows:
 *
 * calculate_weights(GCP, training_data):
	weight_vec = initial_guess()
	grad = avg_gradient(GCP, weight_vec, training_data)
	while (!approx_zero(grad)):
		increment_vec(weight_vec, -LEARNING_RATE * grad)
		grad = avg_gradient(GCP, weight_vec, training_data)
	return weight_vec
 *
 * This method returns a VariableVector with the weights that minimize the loss function.
 */
VariableVector calculate_weights(const string& gcp_filename, const vector<string>& weight_names,
	const vector<string>& partial_names, const vector<pair<VariableVector, VariableVector> >& training_data);


/* This method returns the values of the partial derivatives (the gradient) for a given set of weights,
 *	averaged over the entire set of Training Data.
 * It takes in a VariableVector of weights and a set of Training Data.
 * Recall that training data is provided as a list of {input VariableVector, output VariableVector} pairs.
 * This method repeatedly calls find_partials, which interprets the given GCP to determine the value of the partial derivatives.
 *
 * This method works as follows:
 *
 * avg_gradient(GCP, weight_vector, training_data):
	sum_of_partials = vector_of_zeros()
	for all {input, exp_output} pairs in training_data:
		sum_of_partials += find_partials(GCP, weight_vector, pair)
	return component_wise_div(sum_of_partials, len(training_data))
 *
 * This method is called repeatedly by "calculate_weights", each time with a different set of weights.
 */
VariableVector avg_gradient(const string& gcp_filename, const vector<string>& partial_names,
	const VariableVector& weights, const vector<pair<VariableVector, VariableVector> >& training_data);


/* This method determines the values of the partial derivatives in the given GCP.
 * This method works by running the Interpreter, passing the given set of weights and the given inputs and expected outputs.
 * This method is called repeatedly by avg_gradient, once each for every pair in the Training Data set.
 *
 * find_partials works as follows: 
 *
 * find_partials(GCP, partials, weight_vector, training_data_pair):
	inputs = union(weight_vector, training_data_pair)
	partials = interpret(GCP, inputs)
 *
 * This method returns 0 if the interpretation of the GCP succeeded.
 * Returns an error code otherwise (see utilities.h).
 */
int find_partials(const string& gcp_filename, VariableVector *partials,
				const VariableVector& weights, const VariableVector& inputs,
				const VariableVector& outputs); 



/* ------------------------------------- Helper Functions ------------------------------------- */


/* Returns a VariableVector that is the union of the two given VariableVectors.
 * If the same variable name appears in both vectors, it is only bound to its value from vec1.
 */
const VariableVector variable_vector_union(const VariableVector& vec1, const VariableVector& vec2);

/* Returns a vector. All the names in the given list var_names are bound to the value 0.
 */
VariableVector vector_of_zeros(const vector<string>& var_names);

/* Returns a vector.
 * Every variable in vec1 and every variable in vec2 is in it.
 * If both vec1 and vec2 have a certain variable, the value in the resulting sum vector will be the sum of vec1's value and vec2's value.
 * If only one of vec1 and vec1 have the variable, the value in the resulting sum vector will the value from which vector has it.
 */
VariableVector add_variable_vectors(const VariableVector& vec1, const VariableVector& vec2);

/* Returns a vector.
 * Every variable in vec is in it.
 * The value for each variable is its value in vec divided by the given divisor.
 * Returns NULL if 0 is passed as the divisor.
 */
VariableVector component_wise_div(const VariableVector& vec, float divisor);

/* Returns an initial guess for the weight vector with the given variable names.
 * Currently returns a vector of zeros.
 */
VariableVector initial_weight_guess(const vector<string>& weight_names);

/* Returns a vector.
 * This vector is an updated version of the given weights vector.
 * Each value in the original value is incremented by its counterpart scaled-partial value from the scaled_gradient vector.
 * An empty vector is returned if the weight names don't match up with the partial derivative names.
 */
VariableVector increment_weight_vector(const VariableVector& weights, const VariableVector &scaled_gradient);

/* Returns the name of the weight variable that corresponds to the given partial derivative variable.
 * For example, partial_name_to_weight_name("d/lambda/d/w1") would return "w1".
 * If the partial_name cannot be properly parsed, an empty string is returned.
 */
string partial_name_to_weight_name(const string& partial_name);

/* Returns a vector with each component of the given vec scaled by the given scaling_factor.
 */
VariableVector scale_variable_vector(const VariableVector& vec, float scaling_factor);

/* Returns true if the Pythagorean distance between the given vec and the zero vector is within the constant GRADIENT_PRECISION.
 * GRADIENT_PRECISION is defined at the top of this file.
 * Returns false otherwise.
 */
bool approx_zero(const VariableVector& vec, const vector<string>& partial_names);

/* Returns the Pythagorean distance between the given Variable Vectors.
 * Returns -1 if the vectors do not have identical sets of variables.
 */
float distance_between_variable_vectors(const VariableVector& vec1, const VariableVector& vec2);



void test_vector_of_zeros();
void test_find_partials();
void print_variable_vector(VariableVector v);
void test_add_variable_vectors();
void test_component_wise_div();
void test_avg_gradient();
void test_scale_variable_vector();
void test_approx_zero();
void test_distance_between_variable_vectors();
void test_calculate_weights();

#endif
