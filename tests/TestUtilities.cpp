#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <math.h>

#include "TestUtilities.h"

#define RANDOM_APPROX_PRECISION 0.01

using namespace std;

float logistic(float x) {
	return 1 / (1 + exp(-1 * x));
}


void assert_equal_int(int observed, int expected, const string& test_name) {
	if (observed != expected) {
		cerr << test_name << " -- Observed int: " << observed << " does not equal Expected: " << expected << endl;
		fail(test_name);
		exit(EXIT_FAILURE);
	}
}

void assert_true(bool observed, const string& expression, const string& test_name) {
	if (!observed) {
		cerr << test_name << " -- " << expression << endl;
		fail(test_name);
		exit(EXIT_FAILURE);
	}
}

void assert_false(bool observed, const string& expression, const string& test_name) {
	if (observed) {
		cerr << test_name << " -- " << expression << endl;
		fail(test_name);
		exit(EXIT_FAILURE);
	}
}

void assert_equal_string(string observed, string expected, const string& test_name) {
	if (observed.compare(expected) != 0) {
		cerr << test_name << " -- Observed string: " << observed << " does not equal Expected: " << expected << endl;
		fail(test_name);
		exit(EXIT_FAILURE);
	}
}

void assert_equal_float(float observed, float expected, const string& test_name) {
	if ((observed - expected) > FLOAT_TOLERANCE || (expected - observed) > FLOAT_TOLERANCE) {
		cerr << test_name << " -- Observed float: " << observed << " does not equal Expected: " << expected << endl;
		fail(test_name);
		exit(EXIT_FAILURE);
	}
}

void assert_equal_file_lines(const string& filename, const string expected_lines[], int start_line, int num_lines, const string& test_name) {
	
	ifstream file(filename);
	string line;

	// skip past all the lines before START_LINE
	for (int i = 0; i < start_line; i++) {
		getline(file, line);
	}

	// make sure the next NUM_LINES lines match the expected lines
	for (int j = 0; j < num_lines; j++) {
		getline(file, line);
		assert_equal_string(line, expected_lines[j], test_name);
	}

	file.close();
}

void assert_identical_files(const string& observed_file, const string& expected_file, const string& test_name) {

	ifstream o(observed_file);
	ifstream e(expected_file);

	string o_line, e_line;

	while(!o.eof()) {

		if (e.eof()) {
			cerr << "Observed file " << observed_file << " is longer than expected file " << expected_file << endl;
			fail(test_name); 
		}

        getline(o, o_line);
        getline(e, e_line);

        assert_equal_string(o_line, e_line, test_name);

    }

    if (!e.eof()) {
    	cerr << "Observed file " << observed_file << " is shorter than expected file " << expected_file << endl;
    	fail(test_name);
    }

    o.close();
    e.close();

}

void pass(const string& test_name) {
	cout << "PASS: " << test_name << endl;
}

void fail(const string& test_name) {
	cerr << "FAIL: " << test_name << endl;
}

float generate_approximate(float val) {

	//srand(time(NULL));
	float rand_deviation = (-1 * RANDOM_APPROX_PRECISION) + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2 * RANDOM_APPROX_PRECISION)));
	return val + rand_deviation;
	
}

void assert_less_than(const float observed, const float upper_bound, const string& test_name) {
	if (observed >= upper_bound) {
		cerr << test_name << " -- Observed float: " << observed << " not less than: " << upper_bound << endl;
		fail(test_name);
		exit(EXIT_FAILURE);
	}
}

void assert_greater_than(float observed, float lower_bound, const string& test_name) {
	if (observed <= lower_bound) {
		cerr << test_name << " -- Observed float: " << observed << " not greater than: " << lower_bound << endl;
		fail(test_name);
		exit(EXIT_FAILURE);
	}
}

void assert_approximately_equal_float(float observed, float expected, float error_margin, const string& test_name) {
	if ((observed - expected) > error_margin || (expected - observed) > error_margin) {
		cerr << test_name << " -- Observed float: " << observed << " deviates from Expected: " << expected << " by more than " << error_margin << endl;
		fail(test_name);
		exit(EXIT_FAILURE);
	}
}