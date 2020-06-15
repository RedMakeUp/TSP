/*
	Travelling salesman problem(or TSP)

	Define: Given a list of cities and the distances between each pair of cities, 
	what is the shortest possible route that visits each city and returns to the 
	origin city?(https://en.wikipedia.org/wiki/Travelling_salesman_problem)
*/



#include <iostream>

#include "Common.h"
#include "GA.h"

int main(int argc, char** argv) {
	std::vector<Node> nodes1 = {
		{0.4, 0.4439},    {0.2439 ,0.1463}, {0.1707, 0.2293},
		{0.2293 ,0.761},  {0.5171, 0.9414}, {0.8732, 0.6536},
		{0.6878, 0.5219}, {0.8488, 0.3609}, {0.6683, 0.2536},
		{0.6195 ,0.2634}
	};

	std::vector<Node> nodes2 = {
		{41,94},{37,84},{54,67},{25,62},{7,64},{2,99},{68,58},{71,44},{54,62},{83,69},{64,60},{18,54},{22,60},{83,46},
		{91,38},{25,38},{24,42},{58,69},{71,71},{74,78},{87,76},{18,40},{13,40},{82,7},{62,32},{58,35},{45,21},{41,26},
		{44,35},{4,50}
	};

	auto graph = std::make_shared<GraphT>(nodes1);
	graph->init();

	GA ga(graph);
	ga.solve();

	return 0;
}