// config.cpp

#include <iostream>
#include <string>
#include <map>
#include "config.h"

using namespace std;

bool Config::is_valid() const {
	// { variable : (var, lower) }
	map<string,pair<int,int>>	dict;
	dict["num_main"]		= pair<int,int>(NUM_MAIN_EFFECT_VARIABLES, 0);
	dict["num_second"]		= pair<int,int>(NUM_SECOND_CAUSAL_VARIABLES, 0);
	dict["max_time"]		= pair<int,int>(MAX_TIME, 1);
	dict["max_interval"]	= pair<int,int>(MAX_INVARIANT_TIME, 1);
	dict["num_iterations"]	= pair<int,int>(NUM_ITERATIONS, 1);
	dict["seed"]			= pair<int,int>(SEED, 1);
	dict["num_threads"]		= pair<int,int>(NUM_THREADS, 1);
	
	// check whether the value is less than the lower value
	bool	b = true;
	for(auto p = dict.begin(); p != dict.end(); ++p) {
		const string	variable = p->first;
		const int		value = p->second.first;
		const int		lower = p->second.second;
		if(value < lower) {
			cerr << variable << " (" << value
						<< ") must be greater than or equal to "
						<< lower << "." << endl;
			b = false;
		}
	}
	
	return b;
}

