// Ensemble.cpp

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <chrono>
#include "../include/Ensemble.h"
#include "../include/Predictor.h"
#include "../include/CV.h"
#include "../include/common.h"

using namespace std;
using namespace Statistics;


//////////////////// Ensemble ////////////////////


//////////////////// phenotype and genotype ////////////////////

bool Ensemble::matches_samples(const vector<vector<string>>& pheno_table,
							   const vector<vector<string>>& geno_table) {
	vector<string>	pheno_samples;
	for(auto p = pheno_table.begin() + 1; p != pheno_table.end(); ++p)
		pheno_samples.push_back((*p)[0]);
	const vector<string>&	header = geno_table.front();
	vector<string>	geno_samples = header;
	geno_samples.erase(geno_samples.begin());
	
	if(pheno_samples.size() != geno_samples.size()) {
		cerr << "error : numbers of samples differ." << endl;
		cerr << "        pheno : " << pheno_samples.size() << endl;
		cerr << "        geno  : " << geno_samples.size()  << endl;
		return false;
	}
	
	for(size_t k = 0U; k < pheno_samples.size(); ++k) {
		if(pheno_samples[k] != geno_samples[k]) {
			cerr << "error : pheno and geno samples not match." << endl;
			cerr << "        pheno : " << pheno_samples[k] << endl;
			cerr << "        geno  : " << geno_samples[k] << endl;
			return false;
		}
	}
	
	return true;
}

size_t Ensemble::trait_column(const vector<string>& header,
												const string& trait) {
	for(size_t c = 1U; c != header.size(); ++c) {
		if(header[c] == trait)
			return c;
	}
	return string::npos;
}

Vec Ensemble::get_pheno(const Config& config,
							const vector<vector<string>>& table) {
	const string	trait = config.trait();
	const auto	header = table.front();
	const size_t	c = trait_column(header, trait);
	if(c == string::npos) {
		cerr << "error : trait " << trait << " isn't found." << endl;
		exit(1);
	}
	
	vector<double>	v;
	for(size_t r = 1; r != table.size(); ++r) {
		try {
			v.push_back(stof(table[r][c]));
		}
		catch(runtime_error e) {
			cerr << "error : value " << table[r][c]
									<< " is invalid." << endl;
			exit(1);
		}
	}
	return v;
}

Matrix Ensemble::get_geno(const vector<vector<string>>& table) {
	const size_t	H = table.size();
	const size_t	L = table.front().size();
	Matrix	G(H-1, Vec(L-1));
	// N/A‚ª‚ ‚é‚Æ‚«‚É‘Î‰ž‚Å‚«‚Ä‚¢‚È‚¢
	for(size_t r = 1U; r != H; ++r)
		for(size_t c = 1U; c != L; ++c) {
			G[r-1][c-1] = stoi(table[r][c]);
	}
	return G;
}

vector<string> Ensemble::get_samples(const string& path_pheno) {
	const auto	table = Common::read_csv(path_pheno);
	vector<string>	samples;
	for(size_t i = 1; i < table.size(); ++i)
		samples.push_back(table[i][0]);
	return samples;
}


//////////////////// process ////////////////////

void Ensemble::write_predictions(const Vec& y, const Vec& p,
												const Config& config) {
	ofstream	ofs(config.out().c_str());
	if(!ofs) {
		cerr << "error : can't open " << config.out() << "." << endl;
		exit(1);
	}
	
	const vector<string>	samples = get_samples(config.phenotype());
	ofs << std::setprecision(15);
	ofs << "sample,y,p" << endl;
	for(size_t k = 0U; k < samples.size(); ++k) {
		ofs << samples[k] << "," << y[k] << "," << p[k] << endl;
	}
}

void Ensemble::write_predictions(const Vec& y, const Matrix& pred_table,
														const Config& config) {
	ofstream	ofs(config.out().c_str());
	if(!ofs) {
		cerr << "error : can't open " << config.out() << "." << endl;
		exit(1);
	}
	
	const vector<string>	samples = get_samples(config.phenotype());
	ofs << std::setprecision(15);
	ofs << "smaple,y";
	for(size_t i = 0U; i < pred_table.size(); ++i)
		ofs << ",p" << i+1;
	ofs << endl;
	for(size_t k = 0U; k < samples.size(); ++k) {
		ofs << samples[k];
		ofs << "," << y[k];
		for(size_t i = 0U; i < pred_table.size(); ++i)
			ofs << "," << pred_table[i][k];
		ofs << endl;
	}
}

void Ensemble::print_corr(const Vec& y, const Vec& p) {
	cout << "corr : " << corr(y, p) << endl;
}

void Ensemble::print_corr(const Vec& y, const Matrix& pred_table) {
	const size_t	num_pred = pred_table.size();
	vector<double>	corrs;
	for(auto p = pred_table.begin(); p != pred_table.end(); ++p) {
		corrs.push_back(corr(y, *p));
	}
	vector<double>	ave_pred(y.size());
	for(size_t k = 0U; k != y.size(); ++k) {
		double	s = 0.0;
		for(auto p = pred_table.begin(); p != pred_table.end(); ++p)
			s += (*p)[k];
		ave_pred[k] = s / num_pred;
	}
	
	for(auto p = corrs.begin(); p != corrs.end(); ++p) {
		cout << *p << endl;
	}
	cout << endl;
	cout << corr(y, ave_pred) << endl;
	cout << accumulate(corrs.begin(), corrs.end(), 0.0) / num_pred << endl;
}

void Ensemble::predict_by_self(const Matrix& X, const Vec& y,
												const Config& config) {
	const Predictor	*model = Predictor::make_model(X, y, config);
	const Vec	p = model->predict(X, config.weight_type(),
										config.num_learners(),
										config.weight_exponent());
	delete model;
	write_predictions(y, p, config);
	print_corr(y, p);
}

void Ensemble::predict_CV(const Matrix& X, const Vec& y, const Config& config) {
	const auto	start = chrono::system_clock::now();
	auto	CV = CrossValidation::read(config.path_cv());
	Matrix	pred_table;
	for(size_t k = 0; k < CV.num_repeats(); ++k) {
		Vec	pred(y.size());
		for(int g = 1; g <= CV.num_divisions(); ++g) {
			const Matrix	X_train = CV.extract_X_train(X, k, g);
			const Vec		y_train = CV.extract_y_train(y, k, g);
			const Predictor	*model = Predictor::make_model(
													X_train, y_train, config);
			const Matrix	X_test = CV.extract_X_test(X, k, g);
			const Vec	p_sub = model->predict(X_test, config.weight_type(),
													config.num_learners(),
													config.weight_exponent());
			CV.set_values(pred, p_sub, k, g);
			delete model;
			const auto	end = chrono::system_clock::now();
			const double	elapsed = chrono::duration_cast<chrono::milliseconds>(
															end-start).count();
			cout << k+1 << " " << g << " " << elapsed * 1e-3 << "sec" << endl;
		}
		pred_table.push_back(pred);
	}
	
	write_predictions(y, pred_table, config);
	print_corr(y, pred_table);
}

void Ensemble::process(const Config& config) {
	using namespace Common;
	const vector<vector<string>>	pheno_table = read_csv(config.phenotype());
	const vector<vector<string>>	geno_table = read_csv(config.genotype());
	if(!matches_samples(pheno_table, geno_table))
		return;
	
	Matrix	pred_table;
	const Vec	y = get_pheno(config, pheno_table);
	const Matrix	X = get_geno(geno_table);
	if(config.exists_CV())
		predict_CV(X, y, config);
	else
		predict_by_self(X, y, config);
}
