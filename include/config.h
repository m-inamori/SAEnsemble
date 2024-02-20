#ifndef __INVERSEGRAPH
#define __INVERSEGRAPH


#include <map>
#include <string>
#include <iostream>


//////////////////// ConfigBase ////////////////////

class ConfigBase {
protected:
	std::map<std::string, std::string>	dic;
	
public:
	ConfigBase(const std::string& path);
	
	bool exists_key(const std::string& key) const {
		return dic.find(key) != dic.end();
	}
	
	std::string get(const std::string& key) const {
		auto	p = dic.find(key);
		if(p == dic.end())
			return "";
		else
			return p->second;
	}
	
	void print() const {
		for(auto p = dic.begin(); p != dic.end(); ++p) {
			std::cerr << p->first << " " << p->second << std::endl;
		}
	}
};


//////////////////// ConfigCheckable ////////////////////

class ConfigCheckable : public ConfigBase {
public:
	ConfigCheckable(const std::string& path) : ConfigBase(path) { }
	
	bool check_existence(const std::string& key) const;
	bool check_existence_int(const std::string& key) const;
	bool check_int(const std::string& key) const;
	
	static bool is_int(const std::string& s);
};


//////////////////// Config ////////////////////

class Config : public ConfigCheckable {
public:
	std::string genotype() const	{ return dic.find("GENOTYPE")->second; }
	std::string phenotype() const	{ return dic.find("PHENOTYPE")->second; }
	std::string trait() const		{ return dic.find("TRAIT")->second; }
	std::string path_cv() const		{ return dic.find("PATH_CV")->second; }
	std::string out() const			{ return dic.find("OUTPUT")->second; }
	
	int num_main_vars() const	{ return stoi(dic.find("NUM_MAIN_EFFECT_VARIABLES")->second); }
	int num_second_vars() const	{ return stoi(dic.find("NUM_SECOND_CAUSAL_VARIABLES")->second); }
	int max_time() const		{ return stoi(dic.find("MAX_TIME")->second); }
	int max_invariant() const	{ return stoi(dic.find("MAX_INVARIANT_TIME")->second); }
	int num_iterations() const	{ return stoi(dic.find("NUM_ITERATIONS")->second); }
	int num_learners() const	{ return stoi(dic.find("NUM_LEARNERS")->second); }
	int num_threads() const		{ return stoi(dic.find("NUM_THREADS")->second); }
	
	int seed() const;
	bool exists_seed() const	{ return exists_key("RANDOM_SEED"); }
	bool exists_CV() const		{ return exists_key("PATH_CV"); }
	std::string weight_type() const;
	double weight_exponent() const;
	
public:
	Config(const std::string& path);
};
#endif
