/**
 * @file	urban_mc.cpp
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Sample 4e
 *
 * Cellular automata for urban growth simulation + Genetic Algorithm for calibration
 */

#include "../map.hpp"
using namespace map;
#include <random>


int main(int argc, char **argv)
{	
	std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
    auto randu = [&](){ return dis(gen); };

    const int np = 13;
	typedef std::array<float,np> individual;
	typedef std::vector<individual> population;
	typedef std::vector<float> score;

	//// Arguments

	assert(argc > 12);

	//// Configuration

	setupDevices("",DEV_GPU,"");
	//setupDevices("Intel",DEV_CPU,"");
	//detail::Runtime::getConfig().setNumRanks(1);

	//// Urban

	auto urban = [&](individual indv, int mc, Raster prob)
	{
		//// Inputs

		const float a = indv[0]; //6.4640f;	// Constant coefficient
		const float b1 = indv[1]; //43.5404f;	// Elevation coefficient
		const float b2 = indv[2]; //1.9150f;	// Slope coefficient
		const float b3 = indv[3]; //41.3441f;	// Distance to city centers coefficients
		const float b4 = indv[4]; //12.5878f;	// Distance to transportations coefficient
		std::vector<VariantType> b5 = {0.0f,0.0f,0.0f,indv[6],indv[7],indv[8],indv[9],indv[10],indv[11],indv[12]};
		//std::vector<VariantType> b5 = {0.0f,0.0f,0.0f,-9.8655f,-8.7469f,-9.2688f,-8.0321f,-9.1693f,-8.9420f,-9.4500f};
		// Land use {null,water,urban,barren,forest,shrubland,woody,herbaceous,cultivated,wetlad}
		float d = indv[5]; //5; // dispersion parameter
		int q = 16000; // max # of cells to be converted per it (user defined / from historic data)

		auto x1 = read(argv[1]); // Elevation layer
		auto x2 = read(argv[2]); // Slope layer
		auto x3 = read(argv[3]); // Distance to centers layer
		auto x4 = read(argv[4]); // Distance to transportations layers
		auto x5 = read(argv[5]); // Land use layers
		auto e  = read(argv[6]); // exclusion layer (user defined)
		auto s  = read(argv[7]); // initial urban state
		
		x1 = x1 + zeros()*prob;
		x2 = x2 + zeros()*prob;
		x3 = x3 + zeros()*prob;
		x4 = x4 + zeros()*prob;
		x5 = x5 + zeros()*prob;
		e  = e  + zeros()*prob;
		s  = s  + zeros()*prob;

		const int N = atoi(argv[9]);

		//// Computation
		
		for (int it=0; it<N; it++)
		{
			// Rule 1
			auto z = a + x1*b1 + x2*b2 + x3*b3 + x4*b4 + pick(x5,b5);
			auto pg = 1 / (1 + exp(z));

			// Rule 2
			Array<uint8_t> mask = {1, 1, 1, 1, 0, 1, 1, 1, 1 };
			Mask S = Mask({3,3}, mask);
			auto pc = barrier(pg * !e) * !s * convolve(s,S) / (3*3-1);

			// Rule 3
			auto pd = pc * exp(-d * (1 - pc / max(pc)));

			// Rule 4
			auto ps = q * pd / sum(pd);

			// Random selection
			auto seed = zeros_like(s) + ((it + N*mc) * q + d);
			s = s || ps > rand(seed); // ps > rand() becomes urban
		}

		return s;
	};

	////  Monte Carlo

	auto montecarlo = [&](individual indv)
	{
		auto prob = zeros(); // probability map of turning urban
		const int M = atoi(argv[10]);

		for (int mc=0; mc<M; mc++)
			prob = barrier(prob + urban(indv,mc,prob));
		prob = prob / (float)M;

		return prob;
	};

	////  Genetic Algorithm

	const int P = atoi(argv[11]);
	const int G = atoi(argv[12]);

	std::array<float,np> minv = {0,0,0,0,0,1,-20,-20,-20,-20,-20,-20,-20};
	std::array<float,np> maxv = {50,50,50,50,50,10,0,0,0,0,0,0,0};
	population popu(P), offspring(P);
	score sco(P);

	auto cost = [&](individual indv) {
		auto simulated = montecarlo(indv);
		auto expected = read(argv[8]);
		auto difference = square(simulated - expected);
		auto cost = value(sum(difference)).get<F32>();
		std::cerr << "params: ";
		for (auto &param : indv)
			std::cerr << param << " ";
		std::cerr << " cost: " << cost << std::endl;
		return cost;
	};
	
	auto init = [&]() {
		individual indv;
		for (int i=0; i<np; i++)
			indv[i] = (maxv[i] - minv[i]) * randu() + minv[i];
		return indv;
	};

	auto crossover = [&](individual a, individual b) {
		individual indv;
		for (int i=0; i<np; i++)
			indv[i] = (a[i] + b[i]) / 2.0f;
		return indv;
	};

	auto mutate = [&](individual c) {
		individual indv;
		for (int i=0; i<np; i++) {
			indv[i] = (randu() > 0.9f) ? (maxv[i] - minv[i]) * randu() + minv[i] : c[i];
		}
		return indv;
	};

	auto descend = [&](individual d, int g) {
		individual indv;
		for (int i=0; i<np; i++) {
			float interval = (maxv[i] - minv[i]) * 0.02f*(G-g)/G;
			indv[i] = (randu() > 0.5f) ? d[i] + interval : d[i] - interval;
		}
		return indv;
	};

	auto tournament = [&](population popu, score sco, int K=3) {
		int best = randu() * P;
		for (int k=1; k<K; k++) {
			int i = randu() * P;
			best = sco[i] < sco[best] ? i : best;
		}
		return popu[best];
	};

	auto leader = [&](population popu, score sco) {
		auto best = 0;
		for (int p=1; p<P; p++)
			best = sco[p] < sco[best] ? p : best;
		return popu[best];
	};

	for (int p=0; p<P; p++) {
		popu[p] = init();
		sco[p] = cost(popu[p]);
	}

	for (int g=0; g<G; g++) {
		offspring[0] = leader(popu,sco);
		for (int p=1; p<P; p++) {
			auto a = tournament(popu,sco,P/3);
			auto b = tournament(popu,sco,P/3);
			auto c = crossover(a,b);
			auto d = mutate(c);
			auto e = descend(d,g);
			offspring[p] = e;
		}
		popu = offspring;
		for (int p=0; p<P; p++)
			sco[p] = cost(popu[p]);
	}

	auto best = 0;
	for (int p=1; p<P; p++)
		best = sco[p] < sco[best] ? p : best;
	
	std::cerr << "best params: ";
	for (auto &param : popu[best])
		std::cerr << param << " ";
	std::cerr << " cost: " << sco[best] << std::endl;

	std::cerr << "press any key to exit...";
	std::cin.get();
}
