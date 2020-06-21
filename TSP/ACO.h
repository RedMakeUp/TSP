#pragma once

#include "Common.h"
#include <iostream>
#include <set>

class Ant {
public:
	Ant(FloatType alpha, FloatType beta, std::shared_ptr<GraphT> graph, std::vector<std::vector<FloatType>>& visibility, std::vector<std::vector<FloatType>>& pheromone)
		:m_alpha(alpha), m_beta(beta), g_graph(graph), g_visibility(visibility), g_pheromone(pheromone)
	{
		reset();
	}

	void reset() {
		m_trail.clear();
		m_trail.push_back(0);
		for (size_t i = 1; i < g_graph->getNodeSize(); i++) {
			m_available.insert(i);
		}
	}

	void deposit() {
		FloatType cost = g_graph->getCost(m_trail);
		int Q = 100;
		FloatType depositAmount = Q / cost;
		for (size_t i = 0; i < m_trail.size() - 1;i++) {
			g_pheromone[m_trail[i]][m_trail[i + 1]] += depositAmount;
		}
		g_pheromone[m_trail[m_trail.size() - 1]][m_trail[0]] += depositAmount;
	}

	std::vector<int> stop() {
		deposit();
		auto temp = m_trail;
		reset();
		return temp;
	}

	FloatType moveProbability(int i, int j, FloatType norm) {
		FloatType p = (pow(g_pheromone[i][j], m_alpha)) * (pow(g_visibility[i][j], m_beta));
		p /= norm;
		return p;
	}

	FloatType probabilityNorm(int currentCity) {
		FloatType norm = 0.0;
		for (std::set<int>::iterator i = m_available.begin(); i != m_available.end(); i++) {
			norm += (pow(g_pheromone[currentCity][*i], m_alpha)) * (pow(g_visibility[currentCity][*i], m_beta));
		}
		return norm;
	}

	void step() {
		int currentCity = m_trail.back();
		FloatType norm = probabilityNorm(currentCity);
		FloatType p, gp;
		bool moved = false;
		FloatType highestProb = 0;
		int cityHighest = 0;
		for (std::set<int>::iterator i = m_available.begin(); i != m_available.end(); i++) {
			p = moveProbability(currentCity, *i, norm);
			if (p > highestProb) {
				cityHighest = *i;
				highestProb = p;
			}
			gp = nextFloat();
			if (gp <= p) { // move
				moved = true;
				m_trail.push_back(*i);
				m_available.erase(i);
				break;
			}
		}
		if (!moved) {
			// make a move to the highest available prob city
			// move to cityHighest
			m_trail.push_back(cityHighest);
			m_available.erase(cityHighest);
		}
	}

private:
	std::vector<int> m_trail;
	std::set<int> m_available;
	FloatType m_alpha;
	FloatType m_beta;

	std::shared_ptr<GraphT> g_graph;
	std::vector<std::vector<FloatType>>& g_visibility;
	std::vector<std::vector<FloatType>>& g_pheromone;
};

class ACO : public Solver {
public:
	ACO(std::shared_ptr<GraphT> graph, size_t iterationCount, size_t antCount, FloatType alpha, FloatType beta, FloatType evaporation)
		:Solver(graph), m_iterationCount(iterationCount), m_antCount(antCount),
		m_alpha(alpha), m_beta(beta), m_evaporation(evaporation)
	{}

	Solution solve() override {
		// Initialize visibility and pheromone matrix
		init();

		// Create ants
		for (size_t i = 0; i < m_antCount; i++) {
			m_ants.push_back(Ant(m_alpha, m_beta, m_graph, m_visibility, m_pheromone));
		}

		Route PATH;
		FloatType minCost = std::numeric_limits<FloatType>::max();
		FloatType currentCost = std::numeric_limits<FloatType>::max();
		for (size_t it = 0; it < m_iterationCount; it++) {
			for (size_t p = 0; p < m_graph->getNodeSize() - 1; p++) {
				for (size_t i = 0; i < m_antCount; i++) {
					m_ants[i].step();
				}
			}

			bool isUpdateBest = false;
			for (size_t i = 0; i < m_antCount; i++) {
				auto p = m_ants[i].stop();
				if (!PATH.size()) {
					PATH = p;
					minCost = m_graph->getCost(p);
					continue;
				}
				currentCost = m_graph->getCost(p);
				if (currentCost < minCost) {
					minCost = currentCost;
					PATH = p;
					isUpdateBest = true;
				}
			}

			for (size_t i = 0; i < m_graph->getNodeSize(); i++) {
				for (size_t j = 0; j < m_graph->getNodeSize(); j++) {
					m_pheromone[i][j] *= m_evaporation;
				}
			}

			if(isUpdateBest)
				std::cout << "best cost: " << std::setw(10) << minCost << std::setw(15) << "Iteration: " << it << std::endl;
		}

		return Solution{ PATH , minCost, -1.0};
	}

private:
	void init() {
		m_visibility.resize(m_graph->getNodeSize());
		m_pheromone.resize(m_graph->getNodeSize());

		for (size_t i = 0; i < m_graph->getNodeSize(); i++) {
			m_visibility[i].resize(m_graph->getNodeSize());
			m_pheromone[i].resize(m_graph->getNodeSize());
			
			for (size_t j = 0; j < m_graph->getNodeSize(); j++) {
				if (i == j) continue;
				
				m_pheromone[i][j] = 1.0;
				m_visibility[i][j] = 1.0 / m_graph->getCost(i, j);
			}
		}
	}

private:
	size_t m_iterationCount;
	size_t m_antCount;

	FloatType m_alpha;
	FloatType m_beta;
	FloatType m_evaporation;

	std::vector<std::vector<FloatType>> m_visibility;
	std::vector<std::vector<FloatType>> m_pheromone;

	std::vector<Ant> m_ants;
};