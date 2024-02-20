// config.cpp

#include <iostream>
#include <fstream>
#include <stdexcept>
#include "../include/config.h"
#include "../include/common.h"

using namespace std;


//////////////////// ConfigBase ////////////////////

ConfigBase::ConfigBase(const string& path) {
	ifstream	ifs(path.c_str());
	if(!ifs) {
		cerr << "error : can't open " << path << "." << endl;
		exit(1);
	}
	
	string	line;
	while(getline(ifs, line)) {
		if(line.c_str()[line.length()-1] == '\r')
			line = line.substr(0, line.length()-1);
		if(line.c_str()[0] == '#' || Common::empty_line(line)) {
			continue;
		}
		
		auto	p = line.find('=');
		if(p == string::npos) {
			cerr << "error : invalid line.\n";
			cerr << line << endl;
			exit(1);
		}
		
		const string	key   = Common::strip(line.substr(0, p));
		const string	value = Common::strip(line.substr(p+1));
		dic[key] = value;
	}
}


//////////////////// ConfigCheckable ////////////////////

bool ConfigCheckable::check_existence(const string& key) const {
	if(dic.find(key) == dic.end()) {
		cerr << "key " << key << " must exist." << endl;
		return false;
	}
	return true;
}

bool ConfigCheckable::check_existence_int(const string& key) const {
	auto	q = dic.find(key);
	if(q == dic.end()) {
		cerr << "key " << key << " must exist." << endl;
		return false;
	}
	else if(!ConfigCheckable::is_int(q->second)) {
		cerr << "key " << key << " must be integer." << endl;
		return false;
	}
	return true;
}

bool ConfigCheckable::check_int(const string& key) const {
	auto	q = dic.find(key);
	if(q != dic.end() && !ConfigCheckable::is_int(q->second)) {
		cerr << "key " << key << " must be integer." << endl;
		return false;
	}
	return true;
}

bool ConfigCheckable::is_int(const string& s) {
	try {
		const int	n = stoi(s);
		return true;
	}
	catch(std::invalid_argument e) {
		return false;
	}
}


//////////////////// Config ////////////////////

Config::Config(const string& path) : ConfigCheckable(path) {
	bool	b = true;
	if(!check_existence("GENOTYPE"))	b = false;
	if(!check_existence("PHENOTYPE"))	b = false;
	if(!check_existence("TRAIT"))		b = false;
	if(!check_existence("OUTPUT"))		b = false;
	
	if(!check_existence_int("NUM_MAIN_EFFECT_VARIABLES"))	b = false;
	if(!check_existence_int("NUM_SECOND_CAUSAL_VARIABLES"))	b = false;
	if(!check_existence_int("MAX_TIME"))					b = false;
	if(!check_existence_int("NUM_ITERATIONS"))				b = false;
	if(!check_existence_int("MAX_INVARIANT_TIME"))			b = false;
	if(!check_existence_int("NUM_LEARNERS"))				b = false;
	if(!check_existence_int("NUM_THREADS"))					b = false;
	
	if(check_existence("RANDOM_SEED") && !check_int("RANDOM_SEED"))
		b = false;
	
	if(!b)
		exit(1);
}

int Config::seed() const {
	if(exists_seed())
		return stoi(dic.find("RANDOM_SEED")->second);
	else
		return 2;
}

string Config::weight_type() const {
	const auto	p = dic.find("WEIGHT_TYPE");
	if(p != dic.end())
		return p->second;
	else
		return "rsq";
}

double Config::weight_exponent() const {
	const auto	p = dic.find("WEIGHT_EXP");
	if(p != dic.end())
		return stof(p->second);
	else
		return 1.0;
}
