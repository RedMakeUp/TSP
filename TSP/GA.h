#pragma once

#include "Common.h"
#include <iostream>

class GA :public Solver {
	using Individual = Solution;
	using Population = std::vector<Individual>;

public:
	Solution solve(const Graph& g) override {
		// Initialize population with even size
		Population population(10);
		initPopulation(g, population);

		// Placeholder variables
		Individual best;// The elite
		Individual bestEver;
		Individual v;// Selected randomly by fitness
		Individual offSpring1;
		Individual offSpring2;
		Population offSprings(population.size());

		while (true) {
			// Calculate fitness 
			calcFitness(g, population);
			// Selection and crossover
			findBestIndividual(population, best);
			if (best.fitness > bestEver.fitness) {
				bestEver = best;
				std::cout << bestEver.cost << " " << bestEver.fitness << std::endl;
			}
			for (int i = 0; i < population.size() / 2; i++) {
				selectWithFitness(population, v);
				pmx(best, v, offSpring1, offSpring2);
				offSprings[i * 2] = offSpring1;
				offSprings[i * 2 + 1] = offSpring2;
			}
			// Mutation
			calcFitness(g, offSprings);
			mutate(offSprings);
			// Elite reservation
			calcFitness(g, offSprings);
			Individual worstOffspring;
			int worstIndex = findWorstIndividual(offSprings, worstOffspring);
			offSprings[worstIndex] = best;

			population = offSprings;
		}

		return Solution();
	}

private:
	void initPopulation(const Graph& g, Population& p) {
		Route r;
		for (const auto& city : g) r.push_back(city.first);
		for (size_t i = 0; i < p.size(); i++) {
			p[i].route = r;
			shuffle(p[i].route);
			p[i].cost = -1.0f;
			p[i].fitness = -1.0f;
		}
	}

	void calcFitness(const Graph& g, Population& p) {
		for (size_t i = 0; i < p.size(); i++) {
			p[i].cost = getCost(g, p[i].route);
			p[i].fitness = 1.0f / (p[i].cost);
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
		std::vector<float> prob(p.size());
		float sum = 0.0f;
		for (const auto& v : p) { sum += v.fitness; }
		for (size_t i = 0; i < p.size(); i++) { prob[i] = p[i].fitness / sum; }

		int index = 0;
		float r = nextFloat();

		while (r >= 0 && index < p.size()) {
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
		int loc1 = nextFloat() * parent1.route.size();
		int loc2 = nextFloat() * parent1.route.size();
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
		while (loc < parent1.route.size()) {
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

	void nextGeneration(const Graph& g, Population& p) {
		/*Population newPopulation(p.size());
		Individual v;
		for (size_t i = 0; i < p.size(); i++) {
			pickWithFitness(p, v);
			mutate(g, v);
			newPopulation[i] = v;
		}
		p = newPopulation;*/
	}

	void mutate(Population& offSprings) {
		static const float maxProb = 0.05f;
		static const float minProb = 0.01f;

		float sum = 0.0f;
		float maxFitness = -1.0;
		for (const auto& a : offSprings) {
			sum += a.fitness;
			if (a.fitness > maxFitness) maxFitness = a.fitness;
		}
		float avgFitness = sum / offSprings.size();

		for (size_t i = 0; i < offSprings.size(); i++) {
			auto prob = 1.0f;
			if (offSprings[i].fitness < avgFitness) {
				prob = maxProb;
			}
			else {
				prob = maxProb * (maxProb - minProb) * (offSprings[i].fitness - avgFitness) / (maxFitness - avgFitness);
			}

			if (nextFloat() <= prob) {
				int loc1 = nextFloat() * offSprings[i].route.size();
				int loc2 = nextFloat() * offSprings[i].route.size();

				std::swap(offSprings[i].route[loc1], offSprings[i].route[loc2]);
			}
		}
	}

};