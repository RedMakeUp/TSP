#pragma once

#include <unordered_map>
#include <vector>
#include <limits>
#include <time.h>
#include <random>
#include <algorithm>

#define INF 1000.0f

// Alternative names for more readability
using City = char;
using Route = std::vector<City>;
using Graph = std::unordered_map<City, std::unordered_map<City, float>>;

// Wrapper for route and corresponding cost
struct Solution {
	Route route;
	float cost; 
	float fitness;
};

// Abstract class for all solver(i.e. methods)
class Solver {
protected:
	virtual Solution solve(const Graph& g) = 0;
};

// Get the distance(or cost) between two cities
float getCost(const Graph& g, City city1, City city2) {
	// Return a large value if city1 and city2 not exist in the graph 
	if (g.find(city1) == g.end() || g.find(city2) == g.end()) return std::numeric_limits<float>::max();

	return g.at(city1).at(city2);
}

// Get the distance(or cost) along the route
float getCost(const Graph& g, const Route& route) {
	float sum = 0.0f;

	for (size_t i = 0; i < route.size(); i++) {
		sum += getCost(g, route[i], route[(i + 1) % route.size()]);
	}

	return sum;
}

// Shuffle the order of cities in the route num times
void shuffle(Route& route) {
	for (int i = route.size() - 1; i > 0; i--)
	{
		int j = rand() % (i + 1);

		auto temp = route[i];
		route[i] = route[j];
		route[j] = temp;
	}
}

// Generate a random float value between [0.0, 1.0]
float nextFloat() {
	static std::random_device rd;// Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd());// Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<> dis(0.0, 1.0);
	return dis(gen);
}




// Define a(un)directed weighted graph
const Graph G = {
	{'A', {{'B', 3.0f}, {'C', INF},  {'D', 8.0f},  {'E', 9.0f}}},
	{'B', {{'A', 3.0f}, {'C', 3.0f}, {'D', 10.0f}, {'E', 5.0f}}},
	{'C', {{'A', INF},  {'B', 3.0f}, {'D', 4.0f},  {'E', 3.0f}}},
	{'D', {{'A', 8.0f}, {'B', 10.0f},{'C', 4.0f},  {'E', 20.0f}}},
	{'E', {{'A', 9.0f}, {'B', 5.0f}, {'C', 3.0f},  {'D', 20.0f}}},
};

// Check if the graph is valid
bool validate(const Graph& g) {
	for (const auto& city : g) {
		for (const auto& neighbor : city.second) {
			auto a = g.at(city.first).at(neighbor.first);
			auto b = g.at(neighbor.first).at(city.first);
			if (a < 0 || b < 0 || a != b) {
				return false;
			}
		}
	}
	return true;
}

