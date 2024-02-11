// Predictor.cpp

#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <chrono>
#include <pthread.h>
#include "Predictor.h"
#include "common.h"

using namespace std;
using namespace Statistics;


//////////////////// PredictionParameters ////////////////////

double PredictionParameters::compute_weight(
								const SAResult& result, int i) const {
	if(weight_type == "truncated") {
		return i < num_learners ? 1.0 : 0.0;
	}
	else if(weight_type == "power") {
		// i: from 0
		return pow((double)(i+1), -weight_exponent);
	}
	else if(weight_type == "linear") {
		// 0 -> 1, num_learners -> 0
		return max(0.0, 1.0 - (double)i / num_learners);
	}
	else if(weight_type == "rsq") {
		return pow(result.get_corr(), 2.0);
	}
	else {
		// not come here.
		throw(1);
	}
}

bool PredictionParameters::is_valid_num_learners(int num_iterations,
														int num_learners) {
	if(num_learners <= num_iterations) {
		cerr << "error : num_learners must be less than or equal ";
		cerr << "to num_iterations." << endl;
		return false;
	}
	else if(num_learners <= 0) {
		cerr << "error : num_learners must be positive." << endl;
		return false;
	}
	else {
		return true;
	}
}

bool PredictionParameters::is_valid_prediction_parameters(const Config& config,
											const string& weight_type,
											int num_learners, double w_exp) {
	if(weight_type == "truncated" || weight_type == "linear") {
		return is_valid_num_learners(config.num_iterations(), num_learners);
	}
	else if(weight_type == "power") {
		if(w_exp <= 0.0)
			cerr << "error : w_exp must be positive." << endl;
		return w_exp > 0.0;
	}
	else if(weight_type == "rsq") {
		return true;
	}
	else {
		cerr << "error : weight_type must be truncated or ";
		cerr << "linear or power or req." << endl;
		return false;
	}
}


//////////////////// ConfigThread ////////////////////

ConfigThread::ConfigThread(int k, SimulatedAnnealing *SA_, const Config& config,
											int s) : SA(SA_), offset_seed(s) {
	const int	L = config.num_iterations();
	const int	T = config.num_threads();
	first = k * L / T;
	last = (k + 1) * L / T;
};


//////////////////// Multi Thread ////////////////////

vector<SAResult> process_partialy(int first, int last, int offset_seed,
												SimulatedAnnealing *SA) {
	// スレッドごとに違うSAを作る
	SimulatedAnnealing	*SA2 = SA->copy();
	vector<SAResult>	all_results;
	for(int k = first; k < last; ++k) {
		// 間違っても0を設定しないように奇数にする
		SA2->set_seed((offset_seed + k) * 2 + 1);
		while(true) {
			try {
				all_results.push_back(SA2->process());
				break;
			}
			catch(overflow_error e) {
				continue;
			}
		}
	}
	delete SA2;
	return all_results;
}

void *thread_function(void *config) {
	ConfigThread	*c = (ConfigThread *)config;
	c->results = process_partialy(c->first, c->last, c->offset_seed, c->SA);
	return (void *)NULL;
}


//////////////////// Predictor ////////////////////

Predictor::Predictor(const Matrix& X, const Vec& y,
						const Config *c, int offset_seed) : config(c) {
	const int	T = config->num_threads();
	const int	seed = offset_seed * 2 + 1;	// not zero
	// max_temeratureを一度限り計算する
	SimulatedAnnealing	*SA = new SimulatedAnnealing(X, y, *config, seed);
	SAs.push_back(SA);
	for(int k = 1; k < T; ++k) {
		SAs.push_back(SA->copy());
	}
	
	vector<ConfigThread *>	configs(T);
	for(int k = 0; k < T; ++k) {
		configs[k] = new ConfigThread(k, SAs[k], *config, offset_seed);
	}
	
#ifndef DEBUG
	vector<pthread_t>	threads_t(T);
	for(int k = 0; k < T; ++k) {
		pthread_create(&threads_t[k], NULL, thread_function, (void *)configs[k]);
	}
	
	for(int k = 0; k < T; ++k)
		pthread_join(threads_t[k], NULL);
#else
	for(int k = 0; k < T; ++k) {
		thread_function(configs[k]);
	}
#endif
	
	for(int k = 0; k < T; ++k) {
		const vector<SAResult>&	res = configs[k]->results;
		for(auto p = res.begin(); p != res.end(); ++p)
			results.push_back(*p);
	}
	
	for(int k = 0; k < T; ++k)
		delete configs[k];
	
	sort(results.begin(), results.end());
	reverse(results.begin(), results.end());
}

Predictor::~Predictor() {
	for(auto p = SAs.begin(); p != SAs.end(); ++p)
		delete *p;
}

Vec Predictor::predict(const Matrix& X, const string& weight_type,
						int num_learners, double w_exp) const {
	const PredictionParameters params(weight_type, num_learners, w_exp);
	const size_t	N = X.front().size();
	Vec	p(N, 0.0);
	
	double	sum_weight = 0.0;
	int i = 0;
	for(auto q = results.begin(); q != results.end(); ++q) {
		const double	w = params.compute_weight(*q, i);
		const Vec	v = SAs.front()->predict(*q, X);
		for(size_t k = 0U; k < N; ++k) {
			// SAs are sorted by correlation
			p[k] += w * v[k];
		}
		sum_weight += w;
		++i;
	}
	
	for(auto q = p.begin(); q != p.end(); ++q) {
		*q /= sum_weight;
	}
	
	return p;
}

Vec Predictor::correlations() const {
	Vec	corrs;
	for(auto q = results.rbegin(); q != results.rend(); ++q)
		corrs.push_back(q->get_corr());
	return corrs;
}

vector<const Variables *> Predictor::variables() const {
	vector<const Variables *>	var_table;
	for(auto q = results.rbegin(); q != results.rend(); ++q)
		var_table.push_back(&(q->vars()));
	return var_table;
}

Predictor *Predictor::make_model(const Statistics::Matrix& X,
								const Statistics::Vec& y,
								int num_main, int num_second, int max_time,
								int max_interval, int num_iterations,
								int seed, int num_threads) {
	const Config	*config = new Config(num_main, num_second, max_time,
										max_interval,
										num_iterations, seed, num_threads);
	if(!config->is_valid()) {
		delete config;
		return NULL;
	}
	
	if(X.empty() || X.front().size() != y.size()) {
		cerr << "The number of samples differs from X and y." << endl;
		return NULL;
	}
	
	return new Predictor(X, y, config, seed);
}
