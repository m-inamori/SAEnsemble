#include <Rcpp.h>
#include <map>
#include "Predictor.h"

using namespace Rcpp;
using namespace Statistics;

Vec make_vector(NumericVector v) {
	Vec	w(v.size());
	for(int k = 0; k < v.size(); ++k)
		w[k] = v[k];
	return w;
}

// transpose and convert vector<vector<double>>
Statistics::Matrix make_matrix(NumericMatrix M) {
	Statistics::Matrix	A(M.ncol(), Vec(M.nrow()));
	for(int i = 0; i < M.ncol(); ++i) {
		for(int j = 0; j < M.nrow(); ++j)
			A[i][j] = M(j, i);
	}
	return A;
}

NumericVector R_vector(const Vec& v) {
	NumericVector	w(v.size());
	for(int k = 0; k < (int)v.size(); ++k)
		w[k] = v[k];
	return w;
}

IntegerMatrix R_variables(const Variables& vars) {
	// [ [m, -1, -1] or [m1, m2, And/Or] ]
	IntegerMatrix	M(vars.size(), 3);
	for(int i = 0; i < (int)vars.size(); ++i) {
		const std::vector<int>	v = vars.get_vars(i);
		for(int j = 0; j < 3; ++j)
				M(i, j) = j < (int)v.size() ? v[j] : -1;
	}
	return M;
}

Vec Cpp_vector(const NumericVector& w) {
	Vec	v(w.size());
	for(int k = 0; k < w.size(); ++k)
		v[k] = w[k];
	return v;
}

List R_SAResult(const SAResult& result) {
	return List::create(
		Named("predictions")  = R_vector(result.predictions()),
		Named("coefficients") = R_vector(result.coefficients()),
		Named("observations") = R_vector(result.observations()),
		Named("variables")    = R_variables(result.vars()),
		Named("correlations") = result.get_corr()
	);
}

List R_SAResults(const std::vector<SAResult>& v) {
	List	list(v.size());
	for(int k = 0; k < (int)v.size(); ++k)
		list[k] = R_SAResult(v[k]);
	return list;
}

Variables Cpp_vars(IntegerMatrix M) {
	std::vector<std::vector<int>>	vars;
	for(int i = 0; i < M.nrow(); ++i) {
		if(M(i, 1) == -1) {		// main
			std::vector<int>	v = { M(i, 0) };
			vars.push_back(v);
		}
		else {					// interaction
			std::vector<int>	v = { M(i, 0), M(i, 1), M(i, 2) };
			vars.push_back(v);
		}
	}
	return Variables(vars);
}

SAResult Cpp_SAResult(const List& R_result) {
	IntegerMatrix	vars = as<IntegerMatrix>(R_result["variables"]);
	return SAResult(Cpp_vector(R_result["predictions"]),
					Cpp_vector(R_result["coefficients"]),
					Cpp_vector(R_result["observations"]),
					Cpp_vars(vars));
}

double compute_weight(const SAResult& result, int i,
						const std::string& weight_type,
						int num_learners, double weight_exponent) {
	if(weight_type == "truncated") {
		return i < num_learners ? 1.0 : 0.0;
	}
	else if(weight_type == "power") {
		// i: from 0
		return pow((double)(i+1), -weight_exponent);
	}
	else if(weight_type == "linear") {
		// 0 -> 1, num_learners -> 0
		return std::max(0.0, 1.0 - (double)i / num_learners);
	}
	else if(weight_type == "rsq") {
		return pow(result.get_corr(), 2.0);
	}
	else {
		// not come here.
		throw(1);
	}
}

// [[Rcpp::export]]
List make_model(NumericMatrix X_, NumericVector y_,
								int num_main, int num_second,
								int max_time, int max_interval,
								int num_iterations,
								int seed, int num_threads) {
	Statistics::Matrix	X = make_matrix(X_);
	Vec		y = make_vector(y_);
	auto	*model = Predictor::make_model(X, y,
							num_main, num_second, max_time, max_interval,
							num_iterations, seed, num_threads);
	if(model == NULL)
		return List::create();
	
	std::vector<SAResult>	results = model->get_results();
	List	SAResults = R_SAResults(results);
	delete model;
	return SAResults;
}

// [[Rcpp::export]]
NumericVector predict(List model, NumericMatrix X_,
						std::string weight_type,
						int num_learners, double weight_exp) {
	NumericVector	p(X_.nrow(), 0.0);
	const Statistics::Matrix	X = make_matrix(X_);
	double	sum_weight = 0.0;
	for(int i = 0; i < model.size(); ++i) {
		List	R_result = as<List>(model[i]);
		const SAResult	result = Cpp_SAResult(R_result);
		const double	w = compute_weight(result, i, weight_type,
											num_learners, weight_exp);
		const Vec	p_each = SimulatedAnnealing::predict(result, X);
		for(int k = 0; k < p.size(); ++k)
			p[k] += w * p_each[k];
		sum_weight += w;
	}
	return p / sum_weight;
}
