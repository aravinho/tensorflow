#ifndef TEST_GRADIENTDESCENT_H
#define TEST_GRADIENTDESCENT_H

#include "stdlib.h"

using namespace std;


/* Tests for the Gradient Descent Methods. */

void test_gd_calculate_weights();
void test_gd_avg_gradient();
void test_gd_find_partials();
void test_gd_variable_vector_union();
void test_gd_vector_of_zeros();
void test_gd_add_variable_vectors();
void test_gd_component_wise_div();
void test_gd_increment_weight_vector();
void test_gd_partial_name_to_weight_name();
void test_gd_scale_variable_vector();
void test_gd_approx_zero();
void test_gd_distance_between_variable_vectors();

void run_gd_tests();


#endif
