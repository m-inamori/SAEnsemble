// Ensemble.h

#include "SimulatedAnnealing.h"


//////////////////// Ensemble ////////////////////

class Ensemble {
	const Config&	config;
	std::vector<SAResult>	results;
	
	SimulatedAnnealing	*SA;
	
public:
	Ensemble(const Statistics::Matrix& X,
			 const Statistics::Vec& y, const Config& c);
	~Ensemble();
	
	Statistics::Vec predict(const Statistics::Matrix& X);
	
public:
	static bool matches_samples(const std::vector<std::vector<std::string>>& pheno,
						 const std::vector<std::vector<std::string>>& geno);
	static size_t trait_column(const std::vector<std::string>& header,
												const std::string& trait);
	static Statistics::Vec get_pheno(const Config& config,
						const std::vector<std::vector<std::string>>& table);
	static Statistics::Matrix get_geno(
						const std::vector<std::vector<std::string>>& table);
	static std::vector<std::string> get_samples(const std::string& path_pheno);
	static void write_predictions(const Statistics::Vec& y,
								const Statistics::Vec& p, const Config& config);
	static void write_predictions(const Statistics::Vec& y,
					const Statistics::Matrix& pred_table, const Config& config);
	static void print_corr(const Statistics::Vec& y, const Statistics::Vec& p);
	static void print_corr(const Statistics::Vec& y,
						   const Statistics::Matrix& pred_table);
	
	static void predict_by_self(const Statistics::Matrix& X,
								const Statistics::Vec& y, const Config& c);
	static void predict_CV(const Statistics::Matrix& X,
								const Statistics::Vec& y, const Config& c);
	static void process(const Config& config);
};
