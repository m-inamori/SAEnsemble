#ifndef __ENSEMBLE
#define __ENSEMBLE

#include "SimulatedAnnealing.h"


//////////////////// Ensemble ////////////////////

namespace Ensemble {
	bool matches_samples(const std::vector<std::vector<std::string>>& pheno,
						 const std::vector<std::vector<std::string>>& geno);
	size_t trait_column(const std::vector<std::string>& header,
												const std::string& trait);
	Statistics::Vec get_pheno(const Config& config,
						const std::vector<std::vector<std::string>>& table);
	Statistics::Matrix get_geno(
						const std::vector<std::vector<std::string>>& table);
	std::vector<std::string> get_samples(const std::string& path_pheno);
	void write_predictions(const Statistics::Vec& y,
								const Statistics::Vec& p, const Config& config);
	void write_predictions(const Statistics::Vec& y,
					const Statistics::Matrix& pred_table, const Config& config);
	void print_corr(const Statistics::Vec& y, const Statistics::Vec& p);
	void print_corr(const Statistics::Vec& y,
						   const Statistics::Matrix& pred_table);
	
	void predict_by_self(const Statistics::Matrix& X,
								const Statistics::Vec& y, const Config& c);
	void predict_CV(const Statistics::Matrix& X,
								const Statistics::Vec& y, const Config& c);
	void process(const Config& config);
}
#endif
