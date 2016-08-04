#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

#include <string>

using namespace std;

#define FLOAT_TOLERANCE 0.0001

float logistic(float x);
void assert_equal_int(int observed, int expected, const string& test_name);
void assert_true(bool observed, const string& expression, const string& test_name);
void assert_false(bool observed, const string& expression, const string& test_name);
void assert_equal_string(string observed, string expected, const string& test_name);
void assert_equal_float(float observed, float expected, const string& test_name);
void assert_equal_file_lines(const string& filename, const string expected_lines[], int start_line, int num_lines, const string& test_name);
void assert_identical_files(const string& observed_file, const string& expected_file, const string& test_name);
void pass(const string& test_name);
void fail(const string& test_name);
float generate_approximate(float val);
void assert_less_than(float observed, float upper_bound, const string& test_name);
void assert_greater_than(float observed, float lower_bound, const string& test_name);
void assert_approximately_equal_float(float observed, float expected, float error_margin, const string& test_name);


#endif