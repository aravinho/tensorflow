#include <iostream>
#include <fstream>
#include <cfloat>
#include <time.h>

#include "TestGradientDescent.h"
#include "../src/GradientDescent.h"
#include "TestUtilities.h"

using namespace std;



void test_gd_calculate_weights() {

	// try a simple example
	// the shape program resembles this:
	//	outputs i, j, k = logistic(a*f), logistic(b*g), logistic(c*h)
	//  the loss is avg((i - m)^2, (j - n)^2, (k - p)^2)
	//  the weights are meant to be f = 0.4, g = 0.2, h = 0.1

	// in this example, we use calculate_weights to "learn" the weights f, g and h
	// Our training Data has 10 data points
	// for each data point, the inputs are 1, 2 and 3
	// the outputs are randomly generated approximates of {logistic(0.4), logistic(0.4), logistic(0.3)} 

	// we test to see that the final weights f, g and h are reasonably close to the 0.4, 0.2 and 0.1

	vector<string> weight_names = {"f", "g", "h"};
	vector<string> partial_names = {"d/LAMBDA/d/f", "d/LAMBDA/d/g", "d/LAMBDA/d/h"};
	
	vector<pair<VariableVector, VariableVector> > training_data;
	VariableVector td_input, td_output;

	double a = 1, b = 2, c = 3;
	td_input = {{"a", a}, {"b", b}, {"c", c}};

	double m, n, p;

	// seed the RNG for the generate_approximate function
	srand(time(NULL));

	// build the Training Data set
	for (int i = 0; i < 10; i++) {
		
		// expected outputs are roughly:
		// m = logis(a*f), n = logis(b*g), p = logis(c*h)
		m = generate_approximate(logistic(a * .4));
		n = generate_approximate(logistic(b * .2));
		p = generate_approximate(logistic(c * .10));

		td_output = {{"m", m}, {"n", n}, {"p", p}};

		//add (td_input, td_output) pair to training data
		training_data.push_back(make_pair(td_input, td_output));

	}

	// run the Gradient Descent algorithm
	VariableVector weights = calculate_weights("tests/test_files/inputs/small_net_gcp.tf", weight_names, partial_names, training_data);

	// test to see that the final weights f, g and h are reasonably close to the 0.4, 0.2 and 0.1
	assert_approximately_equal_double(weights.at("f"), 0.4, 0.03, "test_gd_calculate_weights");
	assert_approximately_equal_double(weights.at("g"), 0.2, 0.03, "test_gd_calculate_weights");
	assert_approximately_equal_double(weights.at("h"), 0.1, 0.03, "test_gd_calculate_weights");

	pass("test_gd_calculate_weights");

}

void test_gd_avg_gradient() {

	// try a simple example
	// the shape program resembles this:
	//	outputs i, j, k = logistic(a*f), logistic(b*g), logistic(c*h)
	//  the loss is avg((i - m)^2, (j - n)^2, (k - p)^2)
	//  the weights are meant to be f = 0.4, g = 0.2, h = 0.1

	// in this example, we interpret the GCP with the weights {0.35, 0.24, 0.08}
	// Our training Data has 10 data points
	// for each data point, the inputs are 1, 2 and 3
	// the outputs are randomly generated approximates of {logistic(0.4), logistic(0.4), logistic(0.3)} 

	// we test to see that the average d/LAMBDA/d/f is negative,
	// the average d/LAMBDA/d/g is positive, and the average d/LAMBDA/d/h is negative

	VariableVector weights = {{"f", 0.35}, {"g", .24}, {"h", .08}};
	vector<string> partial_names = {"d/LAMBDA/d/f", "d/LAMBDA/d/g", "d/LAMBDA/d/h"};

	vector<pair<VariableVector, VariableVector> > training_data;
	VariableVector td_input, td_output;

	double a = 1, b = 2, c = 3;
	td_input = {{"a", a}, {"b", b}, {"c", c}};

	double m, n, p;

	// seed the RNG for the generate_approximate function
	srand(time(NULL));

	// build the Training Data set
	for (int i = 0; i < 10; i++) {
		
		// expected outputs are roughly:
		// m = logis(a*f), n = logis(b*g), p = logis(c*h)
		m = generate_approximate(logistic(a * .4));
		n = generate_approximate(logistic(b * .2));
		p = generate_approximate(logistic(c * .10));
		td_output = {{"m", m}, {"n", n}, {"p", p}};

		//add pair to training data
		training_data.push_back(make_pair(td_input, td_output));
	}

	VariableVector avg_grad = avg_gradient("tests/test_files/inputs/small_net_gcp.tf", partial_names, weights, training_data);

	// test to see that d/LAMBDA/d/f is negative, d/LAMBDA/d/g is positive, and d/LAMBDA/d/h is negative
	assert_less_than(avg_grad.at("d/LAMBDA/d/f"), 0, "test_gd_avg_gradient");
	assert_greater_than(avg_grad.at("d/LAMBDA/d/g"), 0, "test_gd_avg_gradient");
	assert_less_than(avg_grad.at("d/LAMBDA/d/h"), 0, "test_gd_avg_gradient");

	pass("test_gd_avg_gradient");
}

void test_gd_find_partials() {
	
	// try a simple example
	// the shape program resembles this:
	//	outputs i, j, k = logistic(a*f), logistic(b*g), logistic(c*h)
	//  the loss is avg((i - m)^2, (j - n)^2, (k - p)^2)
	//  the weights are meant to be f = 0.4, g = 0.2, h = 0.1

	// in this example, we interpret the GCP with the weights {0.35, 0.24, 0.08}
	// the training datum is {(1, logistic(0.4)), (2, logistic(0.4)), (3, logistic(0.3))}

	// we test to see that d/LAMBDA/d/f is negative, d/LAMBDA/d/g is positive, and d/LAMBDA/d/h is negative

	VariableVector weights = {{"f", 0.35}, {"g", 0.24}, {"h", 0.08}};

	double a = 1, b = 2, c = 3;
	VariableVector inputs = {{"a", a}, {"b", b}, {"c", c}};

	double m, n, p;
	m = logistic(1 * .4);
	n = logistic(2 * .2);
	p = logistic(3 * .10);
	VariableVector outputs = {{"m", m}, {"n", n}, {"p", p}};

	VariableVector *partials = new VariableVector();

	assert_equal_int(find_partials("tests/test_files/inputs/small_net_gcp.tf", partials, weights, inputs, outputs), 0, "test_gd_find_partials");
	
	// test to see that d/LAMBDA/d/f is negative, d/LAMBDA/d/g is positive, and d/LAMBDA/d/h is negative
	assert_true(partials->at("d/LAMBDA/d/f") < 0, "d/LAMBDA/d/f should be negative", "test_gd_find_partials");
	assert_true(partials->at("d/LAMBDA/d/g") > 0, "d/LAMBDA/d/g should be positive", "test_gd_find_partials");
	assert_true(partials->at("d/LAMBDA/d/h") < 0, "d/LAMBDA/d/h should be negative", "test_gd_find_partials");

	pass("test_gd_find_partials");

}

void test_gd_variable_vector_union() {
	
	VariableVector empty;
	VariableVector v1 = {{"a", 1}, {"b", -3}, {"c", 5}};
	VariableVector v2 = {{"d", 7}, {"e", 4}, {"f", 0}};
	VariableVector v3 = {{"g", 3}, {"b", 923}, {"h", -23}};

	// two empty VariableVectors
	assert_equal_int(variable_vector_union(empty, empty).size(), 0, "test_gd_variable_vector_union");

	// one empty, one non-empty
	VariableVector empty_union_v1 = variable_vector_union(empty, v1);
	VariableVector v1_union_empty = variable_vector_union(v1, empty);

	assert_equal_int(empty_union_v1.size(), 3, "test_gd_variable_vector_union");
	assert_equal_double(empty_union_v1.at("a"), 1, "test_gd_variable_vector_union");
	assert_equal_double(empty_union_v1.at("b"), -3, "test_gd_variable_vector_union");
	assert_equal_double(empty_union_v1.at("c"), 5, "test_gd_variable_vector_union");
	assert_equal_int(v1_union_empty.size(), 3, "test_gd_variable_vector_union");
	assert_equal_double(v1_union_empty.at("a"), 1, "test_gd_variable_vector_union");
	assert_equal_double(v1_union_empty.at("b"), -3, "test_gd_variable_vector_union");
	assert_equal_double(v1_union_empty.at("c"), 5, "test_gd_variable_vector_union");

	// two non-empty
	VariableVector v1_union_v2 = variable_vector_union(v1, v2);
	VariableVector v2_union_v1 = variable_vector_union(v2, v1);

	assert_equal_int(v1_union_v2.size(), 6, "test_gd_variable_vector_union");
	assert_equal_double(v1_union_v2.at("a"), 1, "test_gd_variable_vector_union");
	assert_equal_double(v1_union_v2.at("b"), -3, "test_gd_variable_vector_union");
	assert_equal_double(v1_union_v2.at("c"), 5, "test_gd_variable_vector_union");
	assert_equal_double(v1_union_v2.at("d"), 7, "test_gd_variable_vector_union");
	assert_equal_double(v1_union_v2.at("e"), 4, "test_gd_variable_vector_union");
	assert_equal_double(v1_union_v2.at("f"), 0, "test_gd_variable_vector_union");

	assert_equal_int(v2_union_v1.size(), 6, "test_gd_variable_vector_union");
	assert_equal_double(v2_union_v1.at("a"), 1, "test_gd_variable_vector_union");
	assert_equal_double(v2_union_v1.at("b"), -3, "test_gd_variable_vector_union");
	assert_equal_double(v2_union_v1.at("c"), 5, "test_gd_variable_vector_union");
	assert_equal_double(v2_union_v1.at("d"), 7, "test_gd_variable_vector_union");
	assert_equal_double(v2_union_v1.at("e"), 4, "test_gd_variable_vector_union");
	assert_equal_double(v2_union_v1.at("f"), 0, "test_gd_variable_vector_union");

	// if the two VariableVectors overlap, an empty VariableVector should be returned
	VariableVector v1_union_v3 = variable_vector_union(v1, v3);
	VariableVector v3_union_v1 = variable_vector_union(v3, v1);
	assert_equal_int(v1_union_v3.size(), 0, "test_gd_variable_vector_union");
	assert_equal_int(v3_union_v1.size(), 0, "test_gd_variable_vector_union");

	pass("test_gd_variable_vector_union");

}

void test_gd_vector_of_zeros() {
	
	// empty vector of names
	vector<string> empty_names_vec;
	VariableVector empty_zero_vec = vector_of_zeros(empty_names_vec);
	assert_equal_int(empty_zero_vec.size(), 0, "test_gd_vector_of_zeros");

	// simple test
	vector<string> var_names;
	var_names.push_back("A");
	var_names.push_back("B");
	var_names.push_back("C");
	var_names.push_back("B");

	VariableVector zeros = vector_of_zeros(var_names);
	assert_equal_int(zeros.size(), 3, "test_gd_vector_of_zeros");
	assert_equal_double(zeros["A"], 0, "test_gd_vector_of_zeros");
	assert_equal_double(zeros["B"], 0, "test_gd_vector_of_zeros");
	assert_equal_double(zeros["C"], 0, "test_gd_vector_of_zeros");
	
	pass("test_gd_vector_of_zeros");

}

void test_gd_add_variable_vectors() {
	
	// add two empty vectors
	VariableVector empty;
	assert_equal_int(add_variable_vectors(empty, empty).size(), 0, "test_gd_add_variable_vectors");

	// add an empty vector to a non-empty vector
	VariableVector v1 = {{"a", 1}, {"b", -3}};
	VariableVector empty_plus_v1 = add_variable_vectors(empty, v1);
	VariableVector v1_plus_empty = add_variable_vectors(v1, empty);
	assert_equal_int(empty_plus_v1.size(), 0, "test_gd_add_variable_vectors");
	assert_equal_int(v1_plus_empty.size(), 0, "test_gd_add_variable_vectors");
	
	// add two vectors of the same size but with different variables
	VariableVector v2 = {{"a", 3}, {"c", 0}};
	VariableVector v1_plus_v2 = add_variable_vectors(v1, v2);
	VariableVector v2_plus_v1 = add_variable_vectors(v2, v1);
	assert_equal_int(v1_plus_v2.size(), 0, "test_gd_add_variable_vectors");
	assert_equal_int(v2_plus_v1.size(), 0, "test_gd_add_variable_vectors");

	// add two non-empty vectors
	VariableVector v3 = {{"a", 2}, {"b", -4}};
	VariableVector v1_plus_v3 = add_variable_vectors(v1, v3);
	VariableVector v3_plus_v1 = add_variable_vectors(v3, v1);

	assert_equal_int(v1_plus_v3.size(), 2, "test_gd_add_variable_vectors");
	assert_equal_int(v3_plus_v1.size(), 2, "test_gd_add_variable_vectors");
	assert_equal_double(v1_plus_v3.at("a"), 3, "test_gd_add_variable_vectors");
	assert_equal_double(v3_plus_v1.at("b"), -7, "test_gd_add_variable_vectors");
	assert_equal_double(v1_plus_v3.at("a"), 3, "test_gd_add_variable_vectors");
	assert_equal_double(v3_plus_v1.at("b"), -7, "test_gd_add_variable_vectors");
	

	pass("test_gd_add_variable_vectors");

}

void test_gd_component_wise_div() {
	
	VariableVector empty;
	VariableVector v = {{"a", 1}, {"b", -3}, {"c", 0}, {"d", 10000}};

	// test empty VariableVector
	assert_equal_int(component_wise_div(empty, 3).size(), 0, "test_gd_component_wise_div");

	// test cannot divide by 0
	assert_equal_int(component_wise_div(v, 0).size(), 0, "test_gd_component_wise_div");

	// simple test
	VariableVector scaled = component_wise_div(v, -2);
	assert_equal_int(scaled.size(), 4, "test_gd_component_wise_div");
	assert_equal_double(scaled.at("a"), -0.5, "test_gd_component_wise_div");
	assert_equal_double(scaled.at("b"), 1.5, "test_gd_component_wise_div");
	assert_equal_double(scaled.at("c"), 0, "test_gd_component_wise_div");
	assert_equal_double(scaled.at("d"), -5000, "test_gd_component_wise_div");

	pass("test_gd_component_wise_div");
}

void test_gd_increment_weight_vector() {
	
	// edge error cases
	VariableVector empty;
	VariableVector weights = {{"a", 3}, {"b", 4}};
	VariableVector partials = {{"d/lambda/d/a", 0.01}, {"d/lambda/d/b", 0.02}, {"d/lambda/d/c", -0.03}};
	assert_equal_int(increment_weight_vector(empty, empty).size(), 0, "test_gd_scale_variable_vector");
	assert_equal_int(increment_weight_vector(empty, partials).size(), 0, "test_gd_scale_variable_vector");
	assert_equal_int(increment_weight_vector(weights, empty).size(), 0, "test_gd_scale_variable_vector");
	assert_equal_int(increment_weight_vector(weights, partials).size(), 0, "test_gd_scale_variable_vector");

	weights["c"] = 5;

	VariableVector incremented = increment_weight_vector(weights, partials);
	assert_equal_int(incremented.size(), 3, "test_gd_scale_variable_vector");
	assert_equal_double(incremented.at("a"), 3.01, "test_gd_scale_variable_vector");
	assert_equal_double(incremented.at("b"), 4.02, "test_gd_scale_variable_vector");
	assert_equal_double(incremented.at("c"), 4.97, "test_gd_scale_variable_vector");

	pass("test_gd_increment_weight_vector");

}

void test_gd_partial_name_to_weight_name() {
	
	assert_equal_string(partial_name_to_weight_name(""), "", "test_gd_partial_name_to_weight_name");
	assert_equal_string(partial_name_to_weight_name("foo"), "", "test_gd_partial_name_to_weight_name");
	assert_equal_string(partial_name_to_weight_name("d/lambda/da"), "", "test_gd_partial_name_to_weight_name");
	assert_equal_string(partial_name_to_weight_name("d/lambda/d/a"), "a", "test_gd_partial_name_to_weight_name");

	pass("test_gd_partial_name_to_weight_name");
}

void test_gd_scale_variable_vector() {

	// test empty VariableVector
	VariableVector empty;
	assert_equal_int(scale_variable_vector(empty, 3).size(), 0, "test_gd_scale_variable_vector");

	// simple test
	VariableVector v = {{"a", 1}, {"b", -3}, {"c", 0}, {"d", 10000}};
	VariableVector scaled = scale_variable_vector(v, -0.01);
	assert_equal_int(scaled.size(), 4, "test_gd_scale_variable_vector");
	assert_equal_double(scaled.at("a"), -0.01, "test_gd_scale_variable_vector");
	assert_equal_double(scaled.at("b"), 0.03, "test_gd_scale_variable_vector");
	assert_equal_double(scaled.at("c"), 0, "test_gd_scale_variable_vector");
	assert_equal_double(scaled.at("d"), -100, "test_gd_scale_variable_vector");

	pass("test_gd_scale_variable_vector");

}

void test_gd_approx_zero() {
	
	VariableVector v1 = {{"a", 0.0001}, {"b", -0.0003}, {"c", 0}};
	VariableVector v2 = {{"a", 0.01}, {"b", -0.3}, {"c", 0.5}};
	VariableVector v3 = {{"a", 0.01}, {"b", -0.3}, {"d", 0.5}};
	vector<string> names = {"a", "b", "c"};

	assert_true(approx_zero(v1, names), "V1 should be approx zero", "test_gd_approx_zero");
	assert_false(approx_zero(v2, names), "V2 should not be approx zero", "test_gd_approx_zero");
	assert_false(approx_zero(v3, names), "V3 should not be approx zero", "test_gd_approx_zero");

	pass("test_gd_approx_zero");

}

void test_gd_distance_between_variable_vectors() {
	
	VariableVector v1 = {{"a", 1}, {"b", -0.3}, {"c", 0}};
	VariableVector v2 = {{"a", 2}, {"b", -0.3}, {"c", -0.5}};
	VariableVector v3 = {{"a", -3}, {"b", 4.1}, {"c", 2}};
	VariableVector v4 = {{"a", 7}, {"b", 6}, {"c", 5}};

	VariableVector empty_vv1 = {};
	VariableVector empty_vv2 = {};

	VariableVector v5 = {{"a", 2}, {"b", -0.3}, {"d", -0.5}};
	VariableVector v6 = {{"a", 2}, {"b", -0.3}, {"c", 10}, {"d", -0.5}};

	// simple errors
	assert_equal_double(distance_between_variable_vectors(v1, v5), -1, "test_gd_distance_between_variable_vectors");
	assert_equal_double(distance_between_variable_vectors(v1, v6), -1, "test_gd_distance_between_variable_vectors");

	// empty VariableVector edge case
	assert_equal_double(distance_between_variable_vectors(empty_vv1, empty_vv2), 0, "test_gd_distance_between_variable_vectors");

	// test the Pythagorean formula is implemented properly
	assert_equal_double(distance_between_variable_vectors(v1, v2), 1.11803, "test_gd_distance_between_variable_vectors");
	assert_equal_double(distance_between_variable_vectors(v1, v3), 6.27375, "test_gd_distance_between_variable_vectors");
	assert_equal_double(distance_between_variable_vectors(v1, v4), 10.03444, "test_gd_distance_between_variable_vectors");

	pass("test_gd_distance_between_variable_vectors");

}


void run_gd_tests() {

	cout << "\nTesting Gradient Descent Methods... " << endl << endl;

	test_gd_calculate_weights();
	test_gd_avg_gradient();
	test_gd_find_partials();
	test_gd_variable_vector_union();
	test_gd_vector_of_zeros();
	test_gd_add_variable_vectors();
	test_gd_component_wise_div();
	test_gd_increment_weight_vector();
	test_gd_partial_name_to_weight_name();
	test_gd_scale_variable_vector();
	test_gd_approx_zero();
	test_gd_distance_between_variable_vectors();

	cout << "\nAll Gradient Descent Tests Passed." << endl << endl;
}

/*test_gd_find_partials -- Observed double: -0.00814073 does not equal Expected: -2.8
test_gd_find_partials -- Observed double: -0.0015479 does not equal Expected: -2.8
test_gd_find_partials -- Observed double: -0.00756581 does not equal Expected: -2.8
test_gd_find_partials -- Observed double: 0.00258278 does not equal Expected: -2.8
test_gd_find_partials -- Observed double: -0.00488492 does not equal Expected: -2.8
test_gd_find_partials -- Observed double: -0.00365385 does not equal Expected: -2.8
test_gd_find_partials -- Observed double: -0.00242277 does not equal Expected: -2.8*/
