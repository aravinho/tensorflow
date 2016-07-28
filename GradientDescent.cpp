#include <vector>
#include "math.h"

#include "GradientDescent.h"
#include "Interpreter.h"
#include "BindingsDictionary.h"
#include "utilities.h"

using namespace std;


VariableVector calculate_weights(const string& gcp_filename, const vector<string>& weight_names, const vector<string>& partial_names, const vector<pair<VariableVector, VariableVector> >& training_data) {

	VariableVector weights = initial_weight_guess(weight_names);
	VariableVector gradient = avg_gradient(gcp_filename, partial_names, weights, training_data);
	cout << "original gradient: ";
	print_variable_vector(gradient);

	cout << "original weights: ";
	print_variable_vector(weights);

	int num_iterations = 0;

	while (!approx_zero(gradient, partial_names) && num_iterations < MAX_NUM_ITERATIONS) {
		weights = increment_weight_vector(weights, scale_variable_vector(gradient, -1 * LEARNING_RATE));
		gradient = avg_gradient(gcp_filename, partial_names, weights, training_data);
		num_iterations++;
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
	float scaled_partial, weight_val;

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

VariableVector scale_variable_vector(const VariableVector& vec, float scaling_factor) {

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
	float pyth_dist = distance_between_variable_vectors(vec, zeros);
	if (pyth_dist == -1) {
		return false;
	}

	return (pyth_dist <= GRADIENT_PRECISION);

}

float distance_between_variable_vectors(const VariableVector& vec1, const VariableVector& vec2) {
	// Both vectors must have the same dimension
	if (vec1.size() != vec2.size()) {
		return -1;
	}

	float square_distance = 0;
	string var_name;
	float val1, val2;

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
	
	VariableVector sum_of_partials = vector_of_zeros(partial_names);

	VariableVector *partials;

	for (vector<pair<VariableVector, VariableVector> >::const_iterator datum = training_data.begin(); datum != training_data.end(); ++datum) {

		partials = new VariableVector();

		VariableVector inputs = datum->first;
		VariableVector outputs = datum->second;
		find_partials(gcp_filename, partials, weights, inputs, outputs);

		sum_of_partials = add_variable_vectors(sum_of_partials, *partials);
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


VariableVector component_wise_div(const VariableVector& vec, float divisor) {

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
	
	VariableVector sum;
	string var_name;
	float component_sum;

	for (VariableVector::const_iterator it1 = vec1.begin(); it1 != vec1.end(); ++it1) {

		var_name = it1->first;

		// if this variable is in both vectors, add the values
		if (vec2.count(var_name) != 0) {
			component_sum = it1->second + vec2.at(var_name);
			sum.insert(make_pair(var_name, component_sum));
		} 
		// if this variable is only in vec1, place the vec1 value in the sum vector
		else {
			sum.insert(make_pair(var_name, it1->second));
		}
	}

	for (VariableVector::const_iterator it2 = vec2.begin(); it2 != vec2.end(); ++it2) {

		var_name = it2->first;

		// if this variable is in both vectors, the previous for loop will have taken care of it
		if (vec1.count(var_name) != 0) {
			continue;
		}
		// if this variable is only in vec2, place the vec2 value in the sum vector
		sum.insert(make_pair(var_name, it2->second));
	}

	return sum;
}

int find_partials(const string& gcp_filename, VariableVector *partials,
				const VariableVector& weights, const VariableVector& inputs,
				const VariableVector& outputs) {

	VariableVector inputs_to_interpreter = variable_vector_union(weights, variable_vector_union(inputs, outputs));

	Interpreter i;
	int success = i.interpret(gcp_filename, inputs_to_interpreter, partials);

	return success;

}

const VariableVector variable_vector_union(const VariableVector& vec1, const VariableVector& vec2) {

	VariableVector v;

	for (VariableVector::const_iterator it1 = vec1.begin(); it1 != vec1.end(); ++it1) {
		v.insert(make_pair(it1->first, it1->second));
	}

	for (VariableVector::const_iterator it2 = vec2.begin(); it2 != vec2.end(); ++it2) {
		if (v.count(it2->first) != 0) {
			continue;
		}
		v.insert(make_pair(it2->first, it2->second));
	}

	return v;
}


void test_vector_of_zeros() {
	vector<string> var_names;
	var_names.push_back("A");
	var_names.push_back("B");
	var_names.push_back("C");

	VariableVector zeros = vector_of_zeros(var_names);
	print_variable_vector(zeros);
}

void test_find_partials() {

	VariableVector weights;
	weights.insert(make_pair("w", 2));
	//weights.insert(make_pair("b", 2));

	VariableVector inputs;
	inputs.insert(make_pair("x", 1));
	//inputs.insert(make_pair("a", 5));

	VariableVector outputs;
	outputs.insert(make_pair("y", 4));

	VariableVector *partials = new VariableVector();

	find_partials("gcp.tf", partials, weights, inputs, outputs);
	print_variable_vector(*partials);

}

void test_add_variable_vectors() {
	VariableVector v1;
	v1.insert(make_pair("a", 1));
	v1.insert(make_pair("b", -3));

	VariableVector v2;
	v2.insert(make_pair("c", 2));
	v2.insert(make_pair("b", -4));
	v2.insert(make_pair("d", 0));

	VariableVector sum = add_variable_vectors(v1, v2);
	print_variable_vector(sum);
}

void test_component_wise_div() {
	VariableVector v2;
	v2.insert(make_pair("c", 2));
	v2.insert(make_pair("b", -4));
	v2.insert(make_pair("d", 0));

	VariableVector q = component_wise_div(v2, -3);
	print_variable_vector(q);

}

void print_variable_vector(VariableVector v) {
	for (VariableVector::iterator it = v.begin(); it != v.end(); ++it) {
		cout << "name: " << it->first << ", val: " << it->second << endl;
	}
}

void test_avg_gradient() {
	vector<string> var_names;
	var_names.push_back("d/lambda/d/w");

	VariableVector weights;
	weights.insert(make_pair("w", 2));

	vector<pair<VariableVector, VariableVector> > training_data;
	VariableVector inputs1 = {{"x", 1}};
	VariableVector inputs2 = {{"x", 2}};
	VariableVector outputs1 = {{"y", 4}};
	VariableVector outputs2 = {{"y", 7}};
	VariableVector inputs3 = {{"x", -3}};
	VariableVector outputs3 = {{"y", -13}};
	
	training_data = {{inputs1, outputs1}, {inputs2, outputs2}, {inputs3, outputs3}};

	VariableVector ag = avg_gradient("gcp.tf", var_names, weights, training_data);
	cout << "avg: " << endl;
	print_variable_vector(ag);
}


void test_scale_variable_vector() {
	VariableVector v = {{"a", 1}, {"b", -3}, {"c", 0}, {"d", 10000}};
	VariableVector s = scale_variable_vector(v, -0.01);
	print_variable_vector(s);		// {a: -0.01, b: 0.03, c: 0, d: -100}
}

void test_approx_zero() {
	VariableVector v1 = {{"a", 0.1}, {"b", -0.3}, {"c", 0}};
	VariableVector v2 = {{"a", 0.01}, {"b", -0.3}, {"c", 0.5}};
	VariableVector v3 = {{"a", 0.01}, {"b", -0.3}, {"d", 0.5}};
	vector<string> names = {"a", "b", "c"};

	// should be approx zero
	if (approx_zero(v1, names)) {
		cout << "V1 Approx zero" << endl;
	} else {
		cout << "V1 NOT Approx zero" << endl;
	}

	// should not be approx zero
	if (approx_zero(v2, names)) {
		cout << "V2 Approx zero" << endl;
	} else {
		cout << "V2 NOT Approx zero" << endl;
	}

	// should not be approx zero (error)
	if (approx_zero(v3, names)) {
		cout << "V3 Approx zero" << endl;
	} else {
		cout << "V3 NOT Approx zero" << endl;
	}
}

void test_distance_between_variable_vectors() {
	VariableVector v1 = {{"a", 1}, {"b", -0.3}, {"c", 0}};
	VariableVector v2 = {{"a", 2}, {"b", -0.3}, {"c", -0.5}};

	cout << "distance between v1 and v2: " << distance_between_variable_vectors(v1, v2) << endl;	// 1.18

	VariableVector v3 = {{"a", 1}, {"b", -0.3}, {"c", 0}};
	VariableVector v4 = {{"a", 2}, {"b", -0.3}, {"d", -0.5}};

	cout << "distance between v3 and v4: " << distance_between_variable_vectors(v3, v4) << endl;	// -1 (error)
}


void test_calculate_weights() {

	vector<string> weight_names = {"w"};
	vector<string> partial_names = {"d/lambda/d/w"};

	vector<pair<VariableVector, VariableVector> > training_data;
	VariableVector inputs1 = {{"x", 1}};
	VariableVector inputs2 = {{"x", 2}};
	VariableVector outputs1 = {{"y", 4}};
	VariableVector outputs2 = {{"y", 7}};
	VariableVector inputs3 = {{"x", -3}};
	VariableVector outputs3 = {{"y", -13}};
	
	training_data = {{inputs1, outputs1}, {inputs2, outputs2}, {inputs3, outputs3}};


	VariableVector weights = calculate_weights("gcp.tf", weight_names, partial_names, training_data);
	print_variable_vector(weights);
}

/*int main(int argc, char *argv[]) {

	//test_find_partials();
	//test_vector_of_zeros();
	//test_add_variable_vectors();
	//test_component_wise_div();
	//test_avg_gradient();
	//test_scale_variable_vector();
	//test_approx_zero();
	//test_distance_between_variable_vectors();
	test_calculate_weights();

	return 0;
}*/