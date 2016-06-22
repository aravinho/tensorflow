tensorflow: Compiler.o Node.o DataFlowGraph.o RunCompiler.o
	g++ Compiler.o Node.o DataFlowGraph.o RunCompiler.o -o tensorflow

RunCompiler.o: RunCompiler.cpp
	g++ -c RunCompiler.cpp

DataFlowGraph.o: DataFlowGraph.cpp DataFlowGraph.h
	g++ -c DataFlowGraph.cpp

Node.o: Node.cpp Node.h
	g++ -c Node.cpp

Compiler.o: Compiler.cpp Compiler.h
	g++ -c Compiler.cpp
