#include <vector>
#include "math.h"

#include "GradientDescent.h"
#include "Interpreter.h"
#include "BindingsDictionary.h"
#include "utilities.h"

using namespace std;


VariableVector calculate_weights(const string& gcp_filename, const vector<string>& weight_names,
	const vector<string>& partial_names, const vector<pair<VariableVector, VariableVector> >& training_data) {

	VariableVector weights = initial_weight_guess(weight_names);
	VariableVector gradient = avg_gradient(gcp_filename, partial_names, weights, training_data);

	int num_iterations = 0;
	cout << "Calculating weights for GCP " << gcp_filename << "..." << endl;
	while (!approx_zero(gradient, partial_names) && num_iterations < MAX_NUM_ITERATIONS) {
		weights = increment_weight_vector(weights, scale_variable_vector(gradient, -1 * LEARNING_RATE));
		gradient = avg_gradient(gcp_filename, partial_names, weights, training_data);
		num_iterations++;
		if (gradient.size() == 0) break;
	}

	return weights;
}

VariableVector initial_weight_guess(const vector<string>& weight_names) {
	return vector_of_zeros(weight_names);
}



VariableVector increment_weight_vector(const VariableVector& weights, const VariableVector& scaled_gradient) {

	VariableVector empty;
	

	if (weights.size() != scaled_gradient.size()) {
		return empty;
	}

	VariableVector incremented;
	string partial_name, weight_name;
	double scaled_partial, weight_val;

	for (VariableVector::const_iterator it = scaled_gradient.begin(); it != scaled_gradient.end(); ++it) {
		partial_name = it->first;
		scaled_partial = it->second;

		weight_name = partial_name_to_weight_name(partial_name);

		if (weights.count(weight_name) == 0) {
			return empty;
		}

		weight_val = weights.at(weight_name);

		incremented.insert(make_pair(weight_name, weight_val + scaled_partial));
	}

	return incremented;
}

string partial_name_to_weight_name(const string& partial_name) {
	/*if (partial_name.compare("") == 0) {
		return "";
	}

	size_t len = partial_name.len();

	size_t prev_slash_index = 0;
	size_t next_slash_index = 0;

	next_slash_index = partial_name.find_first_of("/", 0);
	if (next_slash_index != 1 || partial_name.substr(0, 1).compare("d") != 0) {
		return "";
	}

	prev_slash_index = next_slash_index;
	next_slash_index = partial_name.find_first_of("/", 2);
	if (next_slash_index != 8 || partial_name.substr(2, 8).compare("lambda") != 0) {
		return "";
	}

	prev_slash_index = next_slash_index;
	next_slash_index = partial_name.find_first_of("/", 9);
	if (next_slash_index != 10 || partial_name.substr(9, 10).compare("d") != 0) {
		return "";
	}*/
	if (partial_name.length() < 12) {
		return "";
	}

	string weight_name = partial_name.substr(11);
	return weight_name;
}

VariableVector scale_variable_vector(const VariableVector& vec, double scaling_factor) {

	VariableVector scaled;

	for (VariableVector::const_iterator it = vec.begin(); it != vec.end(); ++it) {
		scaled.insert(make_pair(it->first, it->second * scaling_factor));
	}

	return scaled;
}

bool approx_zero(const VariableVector& vec, const vector<string>& partial_names) {
	if (vec.size() != partial_names.size()) {
		return false;
	}

	VariableVector zeros = vector_of_zeros(partial_names);
	double pyth_dist = distance_between_variable_vectors(vec, zeros);
	if (pyth_dist == -1) {
		return false;
	}

	return (pyth_dist <= GRADIENT_PRECISION);

}

double distance_between_variable_vectors(const VariableVector& vec1, const VariableVector& vec2) {
	// Both vectors must have the same dimension
	if (vec1.size() != vec2.size()) {
		return -1;
	}

	double square_distance = 0;
	string var_name;
	double val1, val2;

	for (VariableVector::const_iterator it = vec1.begin(); it != vec1.end(); ++it) {

		var_name = it->first;

		// Both vectors must have exactly the same variables
		if (vec2.count(var_name) == 0) {
			return -1;
		}

		val1 = vec1.at(var_name);
		val2 = vec2.at(var_name);

		square_distance += pow((val1 - val2), 2);
	}

	return pow(square_distance, 0.5);
}


VariableVector avg_gradient(const string& gcp_filename, const vector<string>& partial_names, const VariableVector& weights, const vector<pair<VariableVector, VariableVector> >& training_data) {
	
	VariableVector empty;
	// check for trivial errors
	if (partial_names.size() != weights.size()) return empty;

	// initialize the sum_of_partials_vector
	VariableVector sum_of_partials = vector_of_zeros(partial_names);

	VariableVector *partials;
	int find_partials_success = 0;


	for (vector<pair<VariableVector, VariableVector> >::const_iterator datum = training_data.begin(); datum != training_data.end(); ++datum) {

		partials = new VariableVector();

		VariableVector inputs = datum->first;
		VariableVector outputs = datum->second;
		
		find_partials_success = find_partials(gcp_filename, partials, weights, inputs, outputs);
		if (find_partials_success != 0) return empty;

		sum_of_partials = add_variable_vectors(sum_of_partials, *partials);
		if (sum_of_partials.size() == 0) return empty;

		delete partials;
	}

	return component_wise_div(sum_of_partials, training_data.size());
}


VariableVector vector_of_zeros(const vector<string>& var_names) {
	VariableVector zeros;
	for (vector<string>::const_iterator name = var_names.begin(); name != var_names.end(); ++name) {
		zeros.insert(make_pair(*name, 0));
	}
	return zeros;
}


VariableVector component_wise_div(const VariableVector& vec, double divisor) {

	VariableVector quotient;

	if (divisor == 0) {
		cerr << "Cannot divide by 0." << endl;
		return quotient;
	}

	for (VariableVector::const_iterator it = vec.begin(); it != vec.end(); ++it) {
		quotient.insert(make_pair(it->first, it->second / divisor));
	}

	return quotient;
}


VariableVector add_variable_vectors(const VariableVector& vec1, const VariableVector& vec2) {
	
	VariableVector empty;
	VariableVector sum;
	string var_name;
	double component_sum;

	// if the vectors are of different sizes, return empty
	if (vec1.size() != vec2.size()) return empty;

	for (VariableVector::const_iterator it1 = vec1.begin(); it1 != vec1.end(); ++it1) {

		var_name = it1->first;

		// if this variable is in both vectors, add the values
		if (vec2.count(var_name) != 0) {
			component_sum = it1->second + vec2.at(var_name);
			sum.insert(make_pair(var_name, component_sum));
		} 
		// if this variable is only in vec1, return empty
		else {
			return empty;
		}
	}

	// if we reach this point, every variable in vec1 is also in vec2
	// since both vectors are of the same size, it is safe to return

	return sum;
}

int find_partials(const string& gcp_filename, VariableVector *partials,
				const VariableVector& weights, const VariableVector& inputs,
				const VariableVector& outputs) {

	// accumulate the inputs to the interpreter
	// make sure there is no overlap between the names of the weights, inputs and output variables
	VariableVector inputs_outputs = variable_vector_union(inputs, outputs);
	if (inputs_outputs.size() == 0) {
		return VAR_DECLARED_TWICE;
	}

	VariableVector inputs_to_interpreter = variable_vector_union(weights, inputs_outputs);
	if (inputs_to_interpreter.size() == 0) {
		return VAR_DECLARED_TWICE;
	}

	Interpreter i;
	int success = i.interpret(gcp_filename, inputs_to_interpreter, partials);

	return success;

}

const VariableVector variable_vector_union(const VariableVector& vec1, const VariableVector& vec2) {

	VariableVector empty;
	VariableVector v;

	// add all the variables in Vec 1 first
	// if any of these variables are seen in Vec 2, this is an error. Return the empty Variable Vector.
	// if there is no overlap, then it is safe to add all the variables in Vec 2
	for (VariableVector::const_iterator it1 = vec1.begin(); it1 != vec1.end(); ++it1) {
		if (vec2.count(it1->first) != 0) {
			return empty;
		}
		v.insert(make_pair(it1->first, it1->second));
	}

	for (VariableVector::const_iterator it2 = vec2.begin(); it2 != vec2.end(); ++it2) {
		v.insert(make_pair(it2->first, it2->second));
	}

	return v;
}

void print_variable_vector(VariableVector v) {
	for (VariableVector::iterator it = v.begin(); it != v.end(); ++it) {
		cout << "name: " << it->first << ", val: " << it->second << endl;
	}
}