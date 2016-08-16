#include <iostream>
#include <cfloat>
#include <string>

#include "BindingsDictionary.h"

using namespace std;


/* ------------------- Constructor/Destructor -------------- */


BindingsDictionary::BindingsDictionary() {

	bindings = new unordered_map<string, double> ();

}


BindingsDictionary::~BindingsDictionary() {
	delete bindings;
}


/* ------------------ Public Methods ---------------------- */


int BindingsDictionary::add_variable(string name) {

	if (has_been_declared(name)) {
		return -1;
	}

	(*bindings)[name] = DBL_MAX;
	return 0;


}

	
int BindingsDictionary::bind_value(string name, double value) {
	
	if (!has_been_declared(name) || has_been_defined(name)) {
		return -1;
	}

	if (value == DBL_MIN || value == DBL_MAX) {
		return -1;
	}

	(*bindings)[name] = value;
	return 0;

}


double BindingsDictionary::get_value(const string& name) const {

	if (!has_been_declared(name)) {
		return DBL_MIN;
	}

	return (*bindings)[name];

}


bool BindingsDictionary::has_been_declared(const string& name) const {
	return (bindings->count(name) != 0);
}


bool BindingsDictionary::has_been_defined(const string& name) const {
	return ((get_value(name) != DBL_MIN) && (get_value(name) != DBL_MAX));
}
