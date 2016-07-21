#include <iostream>
#include "Interpreter.h"

using namespace std;

int main(int argc, char *argv[]) {
	cout << "interpreter" << endl;
	Interpreter i;
	unordered_map<string, float> outputs;
	unordered_map<string, float> inputs;
	inputs.insert(make_pair("x", 1));
	inputs.insert(make_pair("w", 2));
	inputs.insert(make_pair("y", 4));
	i.interpret("shape_prog.tf", inputs, &outputs);

	cout << endl << endl;
	for (unordered_map<string, float>::iterator it = outputs.begin(); it != outputs.end(); ++it) {
		cout << "Var: " << it->first << ", Value: " << it->second << endl;
	}


	return 0;
}
