tensorflow: utilities.o Node.o DataFlowGraph.o Compiler.o RunCompiler.o 
	g++ Compiler.o Node.o DataFlowGraph.o RunCompiler.o utilities.o -o tensorflow

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
