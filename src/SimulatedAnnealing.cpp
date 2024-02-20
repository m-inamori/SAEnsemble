// SimulatedAnnealing.cpp

#include <iostream>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include "../include/SimulatedAnnealing.h"
#include "../include/statistics.h"

using namespace std;
using namespace Statistics;


//////////////////// Status ////////////////////

template<typename T>
bool operator <(const State<T>& result1, const State<T>& result2) {
	return result1.energy() < result2.energy();
}


//////////////////// SAResult ////////////////////

SAResult::SAResult(const Vec& p_, const Vec& b_,
						const Vec& y_, const Variables& vars) :
							p(p_), b(b_), y(y_), variables(vars),
							corr(compute_corr()) { }

double SAResult::compute_corr() {
	return Statistics::corr(p, y);
}

bool operator <(const SAResult& result1, const SAResult& result2) {
	return result1.get_corr() < result2.get_corr();
}


//////////////////// Variables ////////////////////

Variables::Variables(const Matrix *X_, const Vec *y_,
					 int num_main_, int num_second_, Random *r) : X(X_), y(y_),
			num_main_vars(num_main_), num_second_vars(num_second_), random(r) {
	while(true) {
		// repeat if energy method fails
		try {
			vars = create();
			current_X = make_matrix();
			XX = multiply_transpose_and_normal(current_X);
			const double	E = energy();
			break;
		}
		catch(overflow_error e) {
			continue;
		}
	}
}

vector<vector<int>> Variables::create() {
	vector<vector<int>>	vars_;
	const vector<int>	mains = random->sample(0, num_markers(), num_main_vars);
	for(auto p = mains.begin(); p != mains.end(); ++p)
		vars_.push_back(vector<int>(1, *p));
	
	for(int k = 0; k < num_second_vars; ++k)
		vars_.push_back(make_second_variable());
	
	return vars_;
}

vector<int> Variables::make_second_variable() const {
	vector<int>	var = random->sample(0, num_markers(), 2);
	var.push_back(random->randint(0, 2));
	return var;
}

Matrix Variables::make_matrix() const {
	return make_matrix_core(*X);
}

Matrix Variables::make_matrix_core(const Matrix& A) const {
	const size_t	W = A.front().size();
	const size_t	N = vars.size();
	
	Matrix	M(N+1, Vec(W, 1.0));
	for(size_t col = 0U; col != W; ++col)
		M[0][col] = 1.0;
	for(auto p = vars.begin(); p != vars.end(); ++p) {
		const size_t	row = p - vars.begin() + 1;
		const vector<int>&	var = *p;
		if(var.size() == 1U) {
			const int	i = var.front();
			copy(A[i].begin(), A[i].end(), M[row].begin());
		}
		else {
			const Vec	v = make_interaction_variables(var, A);
			copy(v.begin(), v.end(), M[row].begin());
		}
	}
	
	return M;
}

Vec Variables::make_interaction_variables(const vector<int>& var,
											const Matrix& A) const {
	const Vec	v1 = A[var[0]];
	const Vec	v2 = A[var[1]];
	const int	b = var[2];
	Vec	v(v1.size());
	for(size_t k = 0; k < v.size(); ++k) {
		if(b == 1)
			v[k] = v1[k] * v2[k];					// and
		else
			v[k] = v1[k] + v2[k] - v1[k] * v2[k];	// or
	}
	return v;
}

double Variables::energy() const {
	const auto	result = multiple_regression();
	const Vec	y_diff = sub(result.pred, *y);
	double	maximum = y->front();
	double	minimum = y->front();
	for(auto p = y->begin() + 1; p != y->end(); ++p) {
		if(*p > maximum)
			maximum = *p;
		else if(*p < minimum)
			minimum = *p;
	}
	const double	y_width = maximum - minimum;
	return 1e6 * inner_product(y_diff, y_diff) /
							(y_width * y_width * num_strains());
}

RegressionResult Variables::multiple_regression() const {
	return Statistics::multiple_regression_core(XX, current_X, *y);
}

Variables Variables::next() const {
	Variables	new_vars = *this;
	const int	i = random->randint(0, (int)vars.size());
	if(new_vars.vars[i].size() == 1U) {
		new_vars.vars[i] = vector<int>(1, random->randint(0, num_markers()));
	}
	else {
		new_vars.vars[i] = make_second_variable();
	}
//	new_vars.current_X = new_vars.make_matrix();
	new_vars.update_X(i);
	new_vars.update_XX(*this, i);
	return new_vars;
}

void Variables::update_X(int i) {
	const vector<int>&	var = vars[i];
	if(i < num_main_vars) {
		const int	j = var.front();
		copy((*X)[j].begin(), (*X)[j].end(), current_X[i+1].begin());
	}
	else {
		const Vec	v = make_interaction_variables(var, *X);
		copy(v.begin(), v.end(), current_X[i+1].begin());
	}
}

// speed up calculation of X'X
void Variables::update_XX(const Variables& vars, int i) {
	XX[0][i+1] = accumulate(current_X[i+1].begin(), current_X[i+1].end(), 0.0);
	XX[i+1][0] = XX[0][i+1];
	for(int j = 0; j < (int)vars.vars.size(); ++j) {
		XX[i+1][j+1] = inner_product(current_X[i+1], current_X[j+1]);
		XX[j+1][i+1] = XX[i+1][j+1];
	}
}

ostream& operator <<(ostream& os, const Variables& vars) {
	for(int i = 0; i < (int)vars.num_vars(); ++i) {
		const auto&	v = vars.sub_vars(i);
		os << '[' << v.front();
		for(auto p = v.begin() + 1; p != v.end(); ++p)
			os << ", " << *p;
		os << ']';
	}
	return os;
}


//////////////////// SimulatedAnnealing ////////////////////

SimulatedAnnealing::SimulatedAnnealing(const Matrix& X_, const Vec& y_,
												const Config& c, int seed) :
							X(X_), y(y_), config(c), random(new Random(seed)),
							max_temperature(determine_max_temperature()) { }

SimulatedAnnealing::SimulatedAnnealing(const Matrix& X_, const Vec& y_,
								const Config& c, Random *r, double max_t) :
							X(X_), y(y_), config(c), random(r->copy()),
							max_temperature(max_t) { }

SimulatedAnnealing::~SimulatedAnnealing() {
	delete random;
}

SimulatedAnnealing *SimulatedAnnealing::copy() const {
	SimulatedAnnealing	*SA = new SimulatedAnnealing(X, y, config, random,
														max_temperature);
	return SA;
}

void SimulatedAnnealing::set_seed(int seed) {
	random->set_seed(seed);
}

State<Variables> SimulatedAnnealing::initialize_state() const {
	while(true) {
		try {
			auto	vars = Variables(&X, &y, config.num_main_vars(),
											config.num_second_vars(), random);
			return State<Variables>(vars, vars.energy());
		}
		catch(overflow_error e) {
			// retry if singular matrix
			continue;
		}
	}
}

State<Variables> SimulatedAnnealing::next_state(
								const State<Variables>& state) const {
	while(true) {
		try {
			Variables	vars = state.variables().next();
			return State<Variables>(vars, vars.energy());
		}
		catch(overflow_error e) {
			// retry if singular matrix
			continue;
		}
	}
}

SAResult SimulatedAnnealing::process() {
	State<Variables>	state = process_core();
	RegressionResult	result = state.variables().multiple_regression();
	return SAResult(result.pred, result.b, y, state.variables());
}

State<Variables> SimulatedAnnealing::process_core() {
	State<Variables>	prev_state = initialize_state();
	State<Variables>	min_state = prev_state;
	int			invariant = 0;
	for(int t = 0; ; ++t) {
		auto	state = next_state(prev_state);
		if(state.energy() < prev_state.energy()) {
			prev_state = state;
			min_state = min(state, min_state);
			invariant = 0;
		}
		else {
			const double	temp = temperature(t);
			if(temp < 1.0)
				break;
			const double	diff_E = state.energy() - prev_state.energy();
			const double	p = random->random();
			if(p < exp(-diff_E / temp)) {
				prev_state = state;
				invariant = 0;
			}
			else {
				invariant += 1;
				if(invariant == config.max_invariant())
					break;
			}
		}
	}
	
	return min_state;
}


double SimulatedAnnealing::temperature(int t) const {
	return max_temperature * exp(-t*log(max_temperature)/config.max_time());
}

double SimulatedAnnealing::determine_max_temperature() {
	// Determin the maximum temperature by repeating 1000 times
	// so that the probability of transitioning from the initial state
	// to the next state is 1/2 even if the energy increases
	vector<double>	dEs;
	for(int i = 0; i < NUM_ITER; ++i) {
		auto	init_stat = initialize_state();
		auto	next_vars = next_state(init_stat);
		
		const double	dE = next_vars.energy() - init_stat.energy();
		if(dE > 0.0)
			dEs.push_back(dE);
	}
	
	// find the middle energy increasing
	sort(dEs.begin(), dEs.end());
	const double	mid_dE = dEs[dEs.size()/2];
	
	// exp(-dE/T) = 1/2 => dE/log(2)
	return mid_dE / log(2.0);
}

Vec SimulatedAnnealing::predict(const SAResult& result, const Matrix& A) const {
	const Matrix	M = result.vars().make_matrix_core(A);
	const Matrix	T = transpose(M);
	return multiply_Mat_and_Vec(T, result.coefficients());
}
