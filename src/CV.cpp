// CV.cpp

#include <string>
#include <iostream>
#include <fstream>
#include "../include/CV.h"
#include "../include/common.h"

using namespace std;

vector<vector<double>> CrossValidation::extract_X_train(
									const vector<vector<double>>& X,
									size_t k, int g) {
	const vector<int>&	v = table[k];
	vector<vector<double>>	new_X;
	for(size_t i = 0; i < X.size(); ++i) {
		vector<double>	row;
		for(size_t j = 0; j < X[i].size(); ++j) {
			if(v[j] != g)
				row.push_back(X[i][j]);
		}
		new_X.push_back(row);
	}
	return new_X;
}

vector<double> CrossValidation::extract_y_train(const vector<double>& y,
															size_t k, int g) {
	const vector<int>&	v = table[k];
	vector<double>	new_y;
	for(size_t i = 0; i < y.size(); ++i) {
		if(v[i] != g)
			new_y.push_back(y[i]);
	}
	return new_y;
}

vector<vector<double>> CrossValidation::extract_X_test(
									const vector<vector<double>>& X,
									size_t k, int g) {
	const vector<int>&	v = table[k];
	vector<vector<double>>	new_X;
	for(size_t i = 0; i < X.size(); ++i) {
		vector<double>	row;
		for(size_t j = 0; j < X[i].size(); ++j) {
			if(v[j] == g)
				row.push_back(X[i][j]);
		}
		new_X.push_back(row);
	}
	return new_X;
}

void CrossValidation::set_values(vector<double>& pred,
								const vector<double>& p_sub, size_t k, int g) {
	const vector<int>&	v = table[k];
	size_t	j = 0;
	for(size_t i = 0; i < v.size(); ++i) {
		if(v[i] == g) {
			pred[i] = p_sub[j];
			++j;
		}
	}
}

void CrossValidation::write(const string& path) const {
	ofstream	ofs(path.c_str());
	if(!ofs) {
		cerr << path << '.' << endl;
		cerr << "error : can't open " << path << '.' << endl;
		exit(1);
	}
	
	const size_t	nrep = this->num_repeats();
	for(size_t j = 1; j <= nrep; ++j)
		ofs << ",CV" << j;
	ofs << endl;
	for(size_t i = 0; i < samples.size(); ++i) {
		ofs << samples[i];
		for(size_t j = 1; j <= nrep; ++j)
			ofs << ',' << table[i][j];
		ofs << endl;
	}
}

CrossValidation CrossValidation::read(const string& path) {
	const auto		table = Common::read_csv(path);
	const size_t	H = table.size();
	const size_t	W = table.front().size();
	vector<string>	samples(H-1);
	vector<vector<int>>	matrix(W-1, vector<int>(H-1));
	for(size_t j = 0; j < H-1; ++j) {
		samples[j] = table[j+1][0];
		for(size_t i = 0; i < W-1; ++i)
			matrix[i][j] = stoi(table[j+1][i+1]);
	}
	return CrossValidation(samples, matrix);
}
