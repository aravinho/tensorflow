test_objects = TestUtilities.o TestNode.o TestDataFlowGraph.o TestBindingsDictionary.o TestPreprocessor.o TestCompiler.o TestInterpreter.o TestGradientDescent.o
class_objects = DataFlowGraph.o Node.o Compiler.o Preprocessor.o utilities.o Interpreter.o BindingsDictionary.o GradientDescent.o

tensorflow: utilities.o Node.o DataFlowGraph.o Compiler.o RunCompiler.o 
	g++ Compiler.o Node.o DataFlowGraph.o RunCompiler.o utilities.o -o tensorflow

interpreter: utilities.o Interpreter.o BindingsDictionary.o RunInterpreter.o
	g++ utilities.o BindingsDictionary.o Interpreter.o RunInterpreter.o -o interpreter

weighteval: utilities.o BindingsDictionary.o Interpreter.o GradientDescent.o
	g++ utilities.o BindingsDictionary.o Interpreter.o GradientDescent.o -o weighteval

preprocessor: Preprocessor.o utilities.o
	g++ Preprocessor.o utilities.o -o preprocessor

test: $(test_objects) $(class_objects) RunTests.o
	g++ $(test_objects) $(class_objects) RunTests.o -o test

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

TestUtilities.o: tests/TestUtilities.cpp tests/TestUtilities.h
	g++ -c -std=c++11 tests/TestUtilities.cpp

TestNode.o: Node.o tests/TestNode.cpp tests/TestNode.h Node.h
	g++ -c -std=c++11 tests/TestNode.cpp

TestDataFlowGraph.o: DataFlowGraph.o tests/TestDataFlowGraph.cpp tests/TestDataFlowGraph.h
	g++ -c -std=c++11 tests/TestDataFlowGraph.cpp

TestBindingsDictionary.o: BindingsDictionary.o tests/TestBindingsDictionary.cpp tests/TestBindingsDictionary.h
	g++ -c -std=c++11 tests/TestBindingsDictionary.cpp

TestPreprocessor.o: Preprocessor.o tests/TestPreprocessor.cpp tests/TestPreprocessor.h
	g++ -c -std=c++11 tests/TestPreprocessor.cpp

TestCompiler.o: Compiler.o tests/TestCompiler.cpp tests/TestCompiler.h
	g++ -c -std=c++11 tests/TestCompiler.cpp

TestInterpreter.o: Interpreter.o tests/TestInterpreter.cpp tests/TestInterpreter.h
	g++ -c -std=c++11 tests/TestInterpreter.cpp

TestGradientDescent.o: GradientDescent.o tests/TestGradientDescent.cpp tests/TestGradientDescent.h
	g++ -c -std=c++11 tests/TestGradientDescent.cpp

RunTests.o: tests/RunTests.cpp
	g++ -c -std=c++11 tests/RunTests.cpp

