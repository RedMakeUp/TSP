#pragma once

#include "Common.h"
#include <iostream>

class GA :public Solver {
	using Individual = Solution;
	using Population = std::vector<Individual>;

public:
	GA(std::shared_ptr<GraphT> graph, size_t iterationSize, size_t populationSize) 
		:Solver(graph), m_iterationCount(iterationSize), m_populationSize(populationSize)
	{}

	Solution solve() override {
		// Initialize population with even size
		Population population(m_populationSize);
		initPopulation(population);

		// Placeholder variables
		Individual best;// The best individual in each generation
		Individual bestEver;// The best individual from beginning
		Individual v;// Selected randomly by fitness
		Individual offSpring1;
		Individual offSpring2;
		Population offSprings(population.size());
		size_t generation = 0;

		while (generation < m_iterationCount) {
			generation++;

			// Calculate fitness 
			calcFitness(population);
			// Selection and crossover
			findBestIndividual(population, best);
			if (best.fitness > bestEver.fitness) {
				bestEver = best;
				std::cout << "best cost: " << std::setw(10) << bestEver.cost << std::setw(15) << "Iteration: " << generation << std::endl;
			}
			for (size_t i = 0; i < population.size() / 2; i++) {
				selectWithFitness(population, v);
				pmx(best, v, offSpring1, offSpring2);
				offSprings[i * 2] = offSpring1;
				offSprings[i * 2 + 1] = offSpring2;
			}
			// Mutation
			calcFitness(offSprings);
			mutate(offSprings);
			// Evolutionary Reversal
			reversal(offSprings);
			// Elite reservation
			calcFitness(offSprings);
			Individual worstOffspring;
			int worstIndex = findWorstIndividual(offSprings, worstOffspring);
			offSprings[worstIndex] = best;

			population = offSprings;
		}

		return bestEver;
	}

private:
	void initPopulation(Population& p) {
		Route r;
		for (size_t i = 0; i < m_graph->getNodeSize(); i++) r.push_back(i);
		for (size_t i = 0; i < p.size(); i++) {
			p[i].route = r;
			shuffle(p[i].route);
		}
	}

	void calcFitness(Population& p) {
		for (size_t i = 0; i < p.size(); i++) {
			p[i].cost = m_graph->getCost(p[i].route);
			p[i].fitness = 1.0 / (p[i].cost);
		}
	}

	int findBestIndividual(const Population& p, Individual& best) {
		if (p.size() <= 0) return -1;
		if (p.size() == 1) { best = p[0]; return 0; }

		best = p[0];
		int index = 0;
		for (size_t i = 1; i < p.size(); i++) {
			if (p[i].fitness > best.fitness) {
				best = p[i];
				index = i;
			}
		}
		return index;
	}

	int findWorstIndividual(const Population& p, Individual& worst) {
		if (p.size() <= 0) return -1;
		if (p.size() == 1) { worst = p[0]; return 0; }

		worst = p[0];
		int index = 0;
		for (size_t i = 1; i < p.size(); i++) {
			if (p[i].fitness < worst.fitness) {
				worst = p[i];
				index = i;
			}
		}
		return index;
	}

	void selectWithFitness(const Population& p, Individual& v) {
		if (p.size() <= 0) return;
		if (p.size() == 1) { v = p[0]; return; }

		// Normalized fitness
		std::vector<FloatType> prob(p.size());
		FloatType sum = 0.0;
		for (const auto& v : p) { sum += v.fitness; }
		for (size_t i = 0; i < p.size(); i++) { prob[i] = p[i].fitness / sum; }

		int index = 0;
		FloatType r = nextFloat();

		while (r >= 0 && index < static_cast<int>(p.size())) {
			r -= prob[index];
			index++;
		}
		index--;

		v = p[index];
	}

	void pmx(const Individual& parent1, const Individual& parent2, Individual& offspring1, Individual& offspring2) {
		offspring1 = parent1;
		offspring2 = parent2;
		
		// The range of cross probability is 0.4-0.99
		auto prob = nextFloat();
		if (prob < 0.4f || prob > 0.99) return;

		// Generate cross locations loc1 and loc2
		int loc1 = static_cast<int>(nextFloat() * parent1.route.size());
		int loc2 = static_cast<int>(nextFloat() * parent1.route.size());
		if (loc1 > loc2) { std::swap(loc1, loc2); }

		// Hybridize
		int loc = loc1;
		while (loc < loc2) {
			offspring2.route[loc] = parent1.route[loc];
			offspring1.route[loc] = parent2.route[loc];
			loc++;
		}

		// Partially map
		loc = 0;
		int index = -1;
		while (loc < static_cast<int>(parent1.route.size())) {
			if (loc >= loc1 && loc < loc2) {
				loc++;
				continue;
			}

			auto target = parent1.route[loc];
			while (isRepeated(offspring1, target, loc1, loc2, index)) {
				target = offspring2.route[index];
			}
			offspring1.route[loc] = target;

			target = parent2.route[loc];
			while (isRepeated(offspring2, target, loc1, loc2, index)) {
				target = offspring1.route[index];
			}
			offspring2.route[loc] = target;

			loc++;
		}
		
	}

	bool isRepeated(const Individual& v, City target, int minIndex, int maxIndex, int& index) {
		bool repeated = false;
		for (int i = minIndex; i < maxIndex; i++) {
			if (target == v.route[i]) {
				index = i;
				repeated = true;
				break;
			}
		}
		return repeated;
	}

	void mutate(Population& offSprings) {
		static const FloatType maxProb = 0.05;
		static const FloatType minProb = 0.01;

		FloatType sum = 0.0;
		FloatType maxFitness = -1.0;
		for (const auto& a : offSprings) {
			sum += a.fitness;
			if (a.fitness > maxFitness) maxFitness = a.fitness;
		}
		FloatType avgFitness = sum / offSprings.size();

		for (size_t i = 0; i < offSprings.size(); i++) {
			auto prob = 1.0;
			if (offSprings[i].fitness < avgFitness) {
				prob = maxProb;
			}
			else {
				prob = maxProb * (maxProb - minProb) * (offSprings[i].fitness - avgFitness) / (maxFitness - avgFitness);
			}

			if (nextFloat() <= prob) {
				for (int k = 0; k < 0.3 * offSprings[0].route.size(); k++) {
					int loc1 = static_cast<int>(nextFloat() * offSprings[i].route.size());
					int loc2 = static_cast<int>(nextFloat() * offSprings[i].route.size());

					std::swap(offSprings[i].route[loc1], offSprings[i].route[loc2]);
				}
			}
		}
	}

	void reversal(Population& offSprings) {
		for (size_t i = 0; i < offSprings.size(); i++) {
			int loc1 = static_cast<int>(nextFloat() * offSprings[i].route.size());
			int loc2 = static_cast<int>(nextFloat() * offSprings[i].route.size());

			std::swap(offSprings[i].route[loc1], offSprings[i].route[loc2]);
		}
	}

	private:
		size_t m_iterationCount;
		size_t m_populationSize;
};