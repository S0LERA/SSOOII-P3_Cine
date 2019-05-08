compilar:
	g++ ./src/Manager.cpp -o ./src/Manager -pthread -std=c++11

cine:
	./src/Manager

limpiar:
	rm ./src/Manager
