#ifndef GRADIENT_DESCENT_H
#define GRADIENT_DESCENT_H

#include <string>
#include <unordered_map>
#include <vector>

#include "utilities.h"
#include "Interpreter.h"
#include "BindingsDictionary.h"

using namespace std;

#define LEARNING_RATE 0.05
#define MAX_NUM_ITERATIONS 1000
#define GRADIENT_PRECISION 0.5

typedef unordered_map<string, float> VariableVector;

int find_partials(const string& gcp_filename, VariableVector *partials,
				const VariableVector& weights, const VariableVector& inputs,
				const VariableVector& outputs); 

VariableVector avg_gradient(const string& gcp_filename, const vector<string>& partial_names, const VariableVector& weights, const vector<pair<VariableVector, VariableVector> >& training_data);

VariableVector calculate_weights(const string& gcp_filename, const vector<string>& weight_names, const vector<string>& partial_names, const vector<pair<VariableVector, VariableVector> >& training_data);


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
