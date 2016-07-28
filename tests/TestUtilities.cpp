#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "TestUtilities.h"

using namespace std;

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
	if (observed != expected) {
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

void pass(const string& test_name) {
	cout << "PASS: " << test_name << endl;
}

void fail(const string& test_name) {
	cerr << "FAIL: " << test_name << endl;
}