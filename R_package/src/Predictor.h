// Predictor.h

#include "SimulatedAnnealing.h"


//////////////////// PredictionParameters ////////////////////

class PredictionParameters {
	const std::string	weight_type;
	const int			num_learners;
	const double		weight_exponent;
	
public:
	PredictionParameters(const std::string& type, int nl, double w_exp) :
				weight_type(type), num_learners(nl), weight_exponent(w_exp) {
		
	}
	
	double compute_weight(const SAResult& result, int i) const;
	
public:
	static PredictionParameters create(const Config& config,
								const std::string& type, int nl, double w_exp);
	
private:
	static bool is_valid_prediction_parameters(const Config& config,
											const std::string& weight_type,
											int num_learners, double w_exp);
	static bool is_valid_num_learners(int num_iterations, int num_learners);

};


//////////////////// ConfigThread ////////////////////

struct ConfigThread {
	SimulatedAnnealing	*SA;
	int	first;
	int	last;
	int	offset_seed;
	std::vector<SAResult>	results;
	
	ConfigThread(int k, SimulatedAnnealing *SA, const Config& config, int s);
};


//////////////////// Predictor ////////////////////

class Predictor {
	const Config	*config;
	std::vector<SAResult>	results;
	
	std::vector<SimulatedAnnealing *>	SAs;
	
public:
	Predictor(const Statistics::Matrix& X, const Statistics::Vec& y,
										const Config *conf, int offset_seed);
	~Predictor();
	
	Statistics::Vec predict(const Statistics::Matrix& X,
							const std::string& weight_type,
							int num_learners, double w_exp) const;
	Statistics::Vec correlations() const;
	std::vector<const Variables *> variables() const;
	std::vector<SAResult> get_results() const { return results; }
	
	static Predictor *make_model(const Statistics::Matrix& X,
								const Statistics::Vec& y,
								int num_main, int num_second, int max_time,
								int max_interval, int num_iterations,
								int seed, int num_threads);
};
