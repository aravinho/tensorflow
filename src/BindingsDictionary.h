#ifndef BINDINGS_DICTIONARY_H
#define BINDINGS_DICTIONARY_H

#include <string>
#include <unordered_map>

using namespace std;


/* The BindingsDictionary is the main data structure used to interpret TenFlang Programs.
 * It contains bindings between variable names and their values.
 */

class BindingsDictionary {


public:
	unordered_map<string, double> *bindings;


	/* ---------------------- Constructor/Destructor ------------------ */


	/* Constructor.
	 * Initializes bindings to be an empty dictionary.
	 */
	BindingsDictionary();

	/* Destructor.
	 * Deletes bindings map.
	 */
	~BindingsDictionary();


	/* ------------------------ Public Methods ------------------------- */


	/* Creates a dummy {name, DBL_MAX} binding.
	 * If the given name is already in the dictionary, returns -1 and does not create the binding.
	 * Returns 0 on success.
	 */
	int add_variable(string name);

	/* Binds the given value to the given name.
	 * If the given name is not found, returns -1.
	 * If the given variable name has already been defined, returns -1 and does nothing.
	 *	(Variables cannot be redefined.)
	 * If the given value is DBL_MIN or DBL_MAX, returns -1 and does nothing.
	 * DBL_MIN and DBL_MAX are not valid values.
	 * Returns 0 on success.
	 */
	int bind_value(string name, double value);

	/* Returns the value bound to the given name.
	 * If the given name is not found, returns DBL_MIN (negative infinity).
	 */
	double get_value(const string& name) const;

	/* Returns true if a variable with the given name has been declared, and false otherwise.
	 */
	bool has_been_declared(const string& name) const;

	/* Returns true if a variable with the given name has been declared and defined.
	 */
	bool has_been_defined(const string& name) const;


};


#endif
