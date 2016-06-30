#ifndef BINDINGS_DICTIONARY_H
#define BINDINGS_DICTIONARY_H

#include <cfloat>
#include <string>
#include <unordered_map>

using namespace std;


/* The BindingsDictionary is the main data structure used to interpret TenFlang Programs.
 * It contains bindings between variable names and their values.
 */

class BindingsDictionary {


public:
	unordered_map<string, float> *bindings;


	/* Constructor.
	 * Initializes bindings to be an empty dictionary.
	 */
	BindingsDictionary();

	/* Creates a dummy {name, FLT_MAX} binding.
	 * If the given name is already in the dictionary, returns -1 and does not create the binding.
	 * Returns 0 on success.
	 */
	int add_variable(string name);

	/* Binds the given value to the given name.
	 * If the given name is not found, returns -1.
	 * If the given value is FLT_MIN or FLT_MAX, returns -1 and does nothing.
	 * FLT_MIN and FLT_MAX are not valid values.
	 * Returns 0 on success.
	 */
	int bind_value(string name, float value);

	/* Returns the value bound to the given name.
	 * If the given name is not found, returns FLT_MIN (negative infinity).
	 */
	float get_value(const string& name) const;

	/* Returns true if a variable with the given name has been declared, and false otherwise.
	 */
	bool has_been_declared(const string& name) const;

	/* Returns true if a variable with the given name has been declared and defined.
	 */
	bool has_been_defined(const string& name) const;


};


#endif
