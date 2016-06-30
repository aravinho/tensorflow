#include "BindingsDictionary.h"
#include <cfloat>
#include <string>

using namespace std;


BindingsDictionary::BindingsDictionary() {

	bindings = new unordered_map<string, float> ();

}


int BindingsDictionary::add_variable(string name) {

	if (has_been_declared(name)) {
		return -1;
	}

	(*bindings)[name] = FLT_MAX;
	return 0;


}

	
int BindingsDictionary::bind_value(string name, float value) {
	
	if (!has_been_declared(name) || has_been_defined(name)) {
		return -1;
	}

	(*bindings)[name] = value;
	return 0;

}


float BindingsDictionary::get_value(const string& name) const {

	if (!has_been_declared(name)) {
		return FLT_MIN;
	}

	return (*bindings)[name];

}

bool BindingsDictionary::has_been_declared(const string& name) const {
	return (bindings->count(name) != 0);
}

bool BindingsDictionary::has_been_defined(const string& name) const {
	return ((get_value(name) != FLT_MIN) && (get_value(name) != FLT_MAX));
}
