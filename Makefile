Simplify: Simp.o Surface.o SimpVertexClustering.o SimpELEN.o Classes.h common.o
	g++ -O3 -std=c++11 -fopenmp Simp.o common.o Classes.h SimpELEN.o SimpVertexClustering.o Surface.o Vector3f.o -o Simplify

Simp.o: Surface.o Simp.cpp
	g++ -O3 -std=c++11 -c Simp.cpp

SimpVertexClustering.o: Surface.o SimpVertexClustering.cpp SimpVertexClustering.h
	g++ -O3 -std=c++11 -c SimpVertexClustering.cpp

SimpELEN.o: Surface.o SimpELEN.cpp SimpELEN.h
	g++ -O3 -fopenmp -std=c++11 -c SimpELEN.cpp

Surface.o: Surface.h Surface.cpp Vector3f.o
	g++ -O3 -std=c++11 -c Surface.cpp

Vector3f.o: Vector3f.h Vector3f.cpp
	g++ -O3 -std=c++11 -c Vector3f.cpp

common.o: common.h common.cpp
	g++ -O3 -std=c++11 -c common.cpp

clean:
	rm *.o Simplify
