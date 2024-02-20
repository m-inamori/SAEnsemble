#ifndef __SIMULATEDANNEALING
#define __SIMULATEDANNEALING

#include "config.h"
#include "statistics.h"
#include "random.h"


//////////////////// Status ////////////////////

template<typename T>
class State {
	T		vars;
	double	E;
	
public:
	State<T>(const T& vars_, double E_) : vars(vars_), E(E_) { }
	
	const T& variables() const { return vars; }
	const double energy() const { return E; }
};

template<typename T>
bool operator <(const State<T>& result1, const State<T>& result2);


//////////////////// Variables ////////////////////

class Variables {
	const Statistics::Matrix	*X;
	const Statistics::Vec		*y;
	int	num_main_vars;
	int	num_second_vars;
	Random	*random;	// only seed is mutable
	
	std::vector<std::vector<int>>	vars;
	Statistics::Matrix	current_X;
	Statistics::Matrix	XX;
	
public:
	Variables(const Statistics::Matrix *X_, const Statistics::Vec *y_,
							int num_main_, int num_second_, Random *r);
	
	size_t size() const { return vars.size(); }
	const std::vector<int> get_vars(int i) const { return *(vars.begin()+i); }
	const size_t num_strains() const { return y->size(); }
	const size_t num_markers() const { return X->size(); }
	size_t num_vars() const { return vars.size(); }
	std::vector<int> sub_vars(int i) const { return *(vars.begin()+i); }
	
	Statistics::Matrix make_matrix() const;
	Statistics::Matrix make_matrix_core(const Statistics::Matrix& A) const;
	Statistics::Vec make_interaction_variables(const std::vector<int>& var,
											const Statistics::Matrix& A) const;
	double energy() const;
	Statistics::RegressionResult multiple_regression() const;
	Variables next() const;
	
private:
	std::vector<std::vector<int>> create();
	std::vector<int> make_second_variable() const;
	std::vector<std::vector<int>> next_core() const;
	void update_X(int i);
	void update_XX(const Variables& vars, int i);
};

std::ostream& operator <<(std::ostream& os, const Variables& vars);


//////////////////// SAResult ////////////////////

class SAResult {
	Statistics::Vec	p;
	Statistics::Vec	b;
	Statistics::Vec	y;
	Variables		variables;
	double			corr;
	
public:
	SAResult(const std::vector<double>& p_, const std::vector<double>& b_,
			 const std::vector<double>& y_, const Variables& vars);
	
	double get_corr() const { return corr; }
	const Variables& vars() const { return variables; }
	const Statistics::Vec& coefficients() const { return b; }
	
private:
	double compute_corr();
};

bool operator <(const SAResult& result1, const SAResult& result2);


//////////////////// SimulatedAnnealing ////////////////////

class SimulatedAnnealing {
	static const int	NUM_ITER = 1000;	// for determining max temperature
	
	const Statistics::Matrix&	X;
	const Statistics::Vec&		y;
	const Config				config;
	Random		*random;
	const double				max_temperature;

public:
	SimulatedAnnealing(const Statistics::Matrix& X_,
						const Statistics::Vec& y_, const Config& c, int seed);
	SimulatedAnnealing(const Statistics::Matrix& X_, const Statistics::Vec& y_,
									const Config& c, Random *r, double max_t);
	~SimulatedAnnealing();
	SimulatedAnnealing *copy() const;
	void set_seed(int seed);
	
	size_t num_markers() const { return X.size(); }
	size_t num_strains() const { return y.size(); }
	
	State<Variables> initialize_state() const;
	State<Variables> next_state(const State<Variables>& state) const;
	double temperature(int t) const;
	double energy(const Variables& vars) const;
	
	SAResult process();
	
	Statistics::Vec predict(const SAResult& result,
							const Statistics::Matrix& A) const;
	
private:
	State<Variables> process_core();
	double determine_max_temperature();
};
#endif
