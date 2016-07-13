tensorflow: utilities.o Node.o DataFlowGraph.o Compiler.o RunCompiler.o 
	g++ Compiler.o Node.o DataFlowGraph.o RunCompiler.o utilities.o -o tensorflow

interpreter: utilities.o Interpreter.o BindingsDictionary.o RunInterpreter.o
	g++ utilities.o BindingsDictionary.o Interpreter.o RunInterpreter.o -o interpreter

weighteval: utilities.o BindingsDictionary.o Interpreter.o GradientDescent.o
	g++ utilities.o BindingsDictionary.o Interpreter.o GradientDescent.o -o weighteval

preprocessor: Preprocessor.o utilities.o
	g++ Preprocessor.o utilities.o -o preprocessor

Preprocessor.o: Preprocessor.cpp Preprocessor.h utilities.h
	g++ -c -std=c++11 Preprocessor.cpp

RunCompiler.o: RunCompiler.cpp
	g++ -c -std=c++11 RunCompiler.cpp

DataFlowGraph.o: DataFlowGraph.cpp DataFlowGraph.h
	g++ -c -std=c++11 DataFlowGraph.cpp

Node.o: Node.cpp Node.h
	g++ -c -std=c++11 Node.cpp

Compiler.o: Compiler.cpp Compiler.h
	g++ -c -std=c++11 Compiler.cpp

utilities.o: utilities.cpp utilities.h
	g++ -c -std=c++11 utilities.cpp


RunInterpreter.o: RunInterpreter.cpp
	g++ -c -std=c++11 RunInterpreter.cpp

Interpreter.o: Interpreter.cpp Interpreter.h utilities.h BindingsDictionary.h
	g++ -c -std=c++11 Interpreter.cpp

BindingsDictionary.o: BindingsDictionary.cpp BindingsDictionary.h utilities.h
	g++ -c -std=c++11 BindingsDictionary.cpp

GradientDescent.o: GradientDescent.h GradientDescent.cpp
	g++ -c -std=c++11 GradientDescent.cpp

