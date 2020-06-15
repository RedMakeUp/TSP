/*
	Travelling salesman problem(or TSP)

	Define: Given a list of cities and the distances between each pair of cities, 
	what is the shortest possible route that visits each city and returns to the 
	origin city?(https://en.wikipedia.org/wiki/Travelling_salesman_problem)
*/



#include <iostream>

#include "Common.h"
#include "GA.h"

// Print the graph
void print(const Graph& g) {
	for (const auto& city : g) {
		std::cout << "---------------" << std::endl;
		std::cout << city.first << std::endl;
		for (const auto& neighbor : city.second) {
			std::cout << neighbor.first << "-" << neighbor.second << " ";
		}
		std::cout << std::endl;
	}
}
// Print the route
void print(const Route& route) {
	if (route.size() <= 0) return;

	for (size_t i = 0; i < route.size(); i++) {
		std::cout << route[i];
	}
	std::cout << route[0] << std::endl;
}

int main(int argc, char** argv) {
	srand(static_cast<unsigned int>(time(NULL)));

	if (validate(G)) {
		GA ga;
		ga.solve(G);

	}

	return 0;
}