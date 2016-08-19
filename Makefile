test_objects = TestUtilities.o TestNode.o TestDataFlowGraph.o TestBindingsDictionary.o TestPreprocessor.o TestCompiler.o TestInterpreter.o TestGradientDescent.o
src_objects = DataFlowGraph.o Node.o Compiler.o Preprocessor.o utilities.o Interpreter.o BindingsDictionary.o GradientDescent.o
run_objects = RunPreprocessor.o RunCompiler.o RunInterpreter.o RunGradientDescent.o RunTests.o
executables = preprocessor compiler interpreter weighteval

preprocessor_src_objects = Preprocessor.o utilities.o
compiler_src_objects = Node.o DataFlowGraph.o Compiler.o Preprocessor.o utilities.o
interpreter_src_objects = BindingsDictionary.o Interpreter.o Preprocessor.o utilities.o
weighteval_src_objects = $(interpreter_src_objects) GradientDescent.o utilities.o

# Compiler and Linker Flags
CC = g++
CFLAGS = -c -std=c++11
LINKFLAGS = -o


# --------------------- Common Targets -----------------------

# "make" creates all the object files and executables
all: preprocessor compiler interpreter weighteval

# "make clean" removes all object files and executables
clean:
	rm $(test_objects) $(src_objects) $(run_objects) $(executables)



# ------------------- Source Binaries -------------------------

preprocessor: RunPreprocessor.o $(preprocessor_src_objects)
	$(CC) RunPreprocessor.o $(preprocessor_src_objects) $(LINKFLAGS) preprocessor

compiler: RunCompiler.o $(compiler_src_objects)
	$(CC) RunCompiler.o $(compiler_src_objects) $(LINKFLAGS) compiler

interpreter: RunInterpreter.o $(interpreter_src_objects)
	$(CC) RunInterpreter.o $(interpreter_src_objects) $(LINKFLAGS) interpreter

weighteval: RunGradientDescent.o $(weighteval_src_objects)
	$(CC) RunGradientDescent.o $(weighteval_src_objects) $(LINKFLAGS) weighteval



# ----------------------- Test Binaries ------------------------

test: RunTests.o $(test_objects) $(src_objects)
	$(CC) RunTests.o $(test_objects) $(src_objects) $(LINKFLAGS) test



# --------------------- Source Object Files ---------------------

# utilities.h declares utility functions, data types
# and constants used by all parts of the system.
utilities.o: src/utilities.cpp src/utilities.h
	$(CC) $(CFLAGS) src/utilities.cpp


# The Preprocessor expands TenFlang programs.
# Every TenFlang program must be preprocessed
# before it can be compiled or interpreted.
Preprocessor.o: src/Preprocessor.cpp src/Preprocessor.h
	$(CC) $(CFLAGS) src/Preprocessor.cpp

RunPreprocessor.o: src/RunPreprocessor.cpp
	$(CC) $(CFLAGS) src/RunPreprocessor.cpp


# The Node and DataFlowGraph classes are used by the Compiler.
Node.o: src/Node.cpp src/Node.h
	$(CC) $(CFLAGS) src/Node.cpp

DataFlowGraph.o: src/DataFlowGraph.cpp src/DataFlowGraph.h
	$(CC) $(CFLAGS) src/DataFlowGraph.cpp


# The Compiler compiles Shape Programs into Gradient Computing Programs (GCPs).
Compiler.o: src/Compiler.cpp src/Compiler.h
	$(CC) $(CFLAGS) src/Compiler.cpp

RunCompiler.o: src/RunCompiler.cpp
	$(CC) $(CFLAGS) src/RunCompiler.cpp


# The BindingsDictionary class is used by the Interpreter.
BindingsDictionary.o: src/BindingsDictionary.cpp src/BindingsDictionary.h
	$(CC) $(CFLAGS) src/BindingsDictionary.cpp


# The Interpreter interprets and returns the outputs of a TenFlang program.
Interpreter.o: src/Interpreter.cpp src/Interpreter.h
	$(CC) $(CFLAGS) src/Interpreter.cpp

RunInterpreter.o: src/RunInterpreter.cpp
	$(CC) $(CFLAGS) src/RunInterpreter.cpp


# GradientDescent.h declares functions used in the Weight Evaluation Phase.
GradientDescent.o: src/GradientDescent.h src/GradientDescent.cpp
	$(CC) $(CFLAGS) src/GradientDescent.cpp

RunGradientDescent.o: src/RunGradientDescent.cpp
	$(CC) $(CFLAGS) src/RunGradientDescent.cpp



# ------------------------ Test Object Files -------------------- 

# TestUtilities.h defines testing helper functions used by all the test files.
TestUtilities.o: tests/TestUtilities.cpp tests/TestUtilities.h
	$(CC) $(CFLAGS) tests/TestUtilities.cpp

# Every class has its own test file.
TestNode.o: tests/TestNode.cpp tests/TestNode.h
	$(CC) $(CFLAGS) tests/TestNode.cpp

TestDataFlowGraph.o: tests/TestDataFlowGraph.cpp tests/TestDataFlowGraph.h
	$(CC) $(CFLAGS) tests/TestDataFlowGraph.cpp

TestBindingsDictionary.o: tests/TestBindingsDictionary.cpp tests/TestBindingsDictionary.h
	$(CC) $(CFLAGS) tests/TestBindingsDictionary.cpp

TestPreprocessor.o: tests/TestPreprocessor.cpp tests/TestPreprocessor.h
	$(CC) $(CFLAGS) tests/TestPreprocessor.cpp

TestCompiler.o: tests/TestCompiler.cpp tests/TestCompiler.h
	$(CC) $(CFLAGS) tests/TestCompiler.cpp

TestInterpreter.o: tests/TestInterpreter.cpp tests/TestInterpreter.h
	$(CC) $(CFLAGS) tests/TestInterpreter.cpp

TestGradientDescent.o: tests/TestGradientDescent.cpp tests/TestGradientDescent.h
	$(CC) $(CFLAGS) tests/TestGradientDescent.cpp

# RunTest.cpp runs all of the tests.
RunTests.o: tests/RunTests.cpp
	$(CC) $(CFLAGS) tests/RunTests.cpp

