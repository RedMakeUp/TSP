#pragma once

#include <unordered_map>
#include <vector>
#include <limits>
#include <time.h>
#include <random>
#include <algorithm>
#include <memory>
#include <iomanip>

// Alternative names for more readability
using City = int;
using Route = std::vector<City>;
using FloatType = double;

struct Node {
	FloatType x;
	FloatType y;
};

FloatType getDistanceSquare(const Node& n1, const Node& n2) {
	auto dx = n1.x - n2.x;
	auto dy = n1.y - n2.y;
	return dx * dx + dy * dy;
}

FloatType getDistance(const Node& n1, const Node& n2) {
	return std::sqrt(getDistanceSquare(n1,n2));
}


class GraphT {
public:
	GraphT(const std::vector<Node>& nodes) : m_nodes(nodes) {}

	void init() {
		for (size_t i = 0; i < m_nodes.size(); i++) {
			m_distances[i].resize(m_nodes.size());

			for (size_t j = 0; j < m_nodes.size(); j++) {
				// Use getDistanceSquare to speed up a little 
				m_distances[i][j] = getDistance(m_nodes[i], m_nodes[j]);
			}
		}
	}

	FloatType getCost(int index1, int index2) { return m_distances[index1][index2]; }

	FloatType getCost(const std::vector<int>& route) {
		FloatType sum = 0.0;
		for (size_t i = 0; i < route.size(); i++) {
			sum += getCost(route[i], route[(i + 1) % route.size()]);
		}
		return sum;
	}

	inline size_t getNodeSize() const { return m_nodes.size(); }

private:
	std::vector<Node> m_nodes;
	std::unordered_map<int, std::vector<FloatType>> m_distances;
};

// Wrapper for route and corresponding cost
struct Solution {
	Route route;
	FloatType cost = 100000.0;
	FloatType fitness = -1.0;
};

// Abstract class for all solver(i.e. methods)
class Solver {
protected:
	Solver(std::shared_ptr<GraphT> graph) :m_graph(graph) {}
	virtual Solution solve() = 0;

	std::shared_ptr<GraphT> m_graph = nullptr;
};

// Shuffle the order of cities in the route num times
void shuffle(Route& route) {
	for (int i = route.size() - 1; i > 0; i--)
	{
		int j = rand() % (i + 1);
		std::swap(route[i], route[j]);
	}
}

// Generate a random float value between [0.0, 1.0)
FloatType nextFloat() {
	static std::random_device rd;// Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd());// Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<> dis(0.0, 1.0);
	return static_cast<FloatType>(dis(gen));
}

// Return the index of specified city in a route, return -1 if the city not exist
int findIndex(const Route& route, City city) {
	int index = -1;

	for (size_t i = 0; i < route.size(); i++) {
		if (city == route[i]) {
			index = i;
			break;
		}
	}

	return index;
}

