#pragma once

#include "Common.h"
#include <iostream>

class PSO :public Solver {
	struct SwapOperator{
		int swap_a = -1;
		int swap_b = -1;
		FloatType prob = 1.0;
	};

	using Velocity = std::vector<SwapOperator>;
	struct Particle {
		Solution current;// Current solution
		Solution best;// Best solution it has achieved so far
		Velocity velocity;
	};
public:
	PSO(std::shared_ptr<GraphT> graph, size_t iterationCount, size_t particleCount, FloatType beta= 1.0, FloatType alpha=1.0) 
		:Solver(graph), 
		m_iterationCount(iterationCount), m_particleCount(particleCount),
		m_beta(beta), m_alpha(alpha)
	{}

	Solution solve() override {
		m_particles.resize(m_particleCount);
		initParticles();

		for (size_t it = 0; it < m_iterationCount; it++) {
			bool isUpdate = updateGlobalBestParticle();
			if(isUpdate)
				std::cout << "best cost: " << std::setw(10) << m_globalBest.best.cost << std::setw(15) << "Iteration: " << it << std::endl;


			for (auto& particle : m_particles) {
				particle.velocity.clear();

				Velocity tempVelocity;
				Solution gBestInd = m_globalBest.best;
				Solution pBestInd = particle.best;
				Solution particleInd = particle.current;

				// Generates all swap operators to calculate (pbest - x(t-1))
				for (size_t i = 0; i < m_graph->getNodeSize(); i++) {
					if (particleInd.route[i] != pBestInd.route[i]) {
						SwapOperator so;
						so.swap_a = i;
						so.swap_b = findIndex(pBestInd.route, particleInd.route[i]);
						so.prob = m_alpha;

						tempVelocity.push_back(so);

						std::swap(pBestInd.route[so.swap_a], pBestInd.route[so.swap_b]);
					}
				}

				// Generates all swap operators to calculate (gbest - x(t-1))
				for (size_t i = 0; i < m_graph->getNodeSize(); i++) {
					if (particleInd.route[i] != gBestInd.route[i]) {
						SwapOperator so;
						so.swap_a = i;
						so.swap_b = findIndex(gBestInd.route, particleInd.route[i]);
						so.prob = m_beta;

						tempVelocity.push_back(so);

						std::swap(gBestInd.route[so.swap_a], gBestInd.route[so.swap_b]);
					}
				}

				// Update velocity
				particle.velocity = tempVelocity;

				// Generate new solution for particle
				for (size_t i = 0; i < tempVelocity.size(); i++) {
					auto& so = tempVelocity[i];
					if (nextFloat() <= so.prob) {
						std::swap(particleInd.route[so.swap_a], particleInd.route[so.swap_b]);
					}
				}

				// Update the current solution
				particle.current = particleInd;
				particle.current.cost = m_graph->getCost(particle.current.route);

				// Checks if current solution is pBest solution
				if (particle.current.cost < particle.best.cost) {
					particle.best = particle.current;
				}
			}
		}

		return m_globalBest.best;
	}

private:
	void initParticles() {
		Route r;
		for (size_t i = 0; i < m_graph->getNodeSize(); i++) r.push_back(i);
		for (size_t i = 0; i < m_particles.size(); i++) {
			Route route = r;
			shuffle(route);
			Solution v;
			v.route = route;
			v.cost = m_graph->getCost(v.route);
			
			m_particles[i].current = v;
			m_particles[i].best = v;
		}
	}

	bool updateGlobalBestParticle() {
		bool isUpdate = false;
		for (const auto& particle : m_particles) {
			if (particle.best.cost < m_globalBest.best.cost) {
				m_globalBest = particle;
				isUpdate = true;
			}
		}

		return isUpdate;
	}

private:
	size_t m_iterationCount;
	size_t m_particleCount;
	FloatType m_alpha;
	FloatType m_beta;
	std::vector<Particle> m_particles;
	Particle m_globalBest;
};