// predict_ensenmble.cpp

#include <iostream>
#include "../include/Ensemble.h"
#include "../include/common.h"

using namespace std;
using namespace Statistics;


//////////////////// process ////////////////////


//////////////////// main ////////////////////

int main(int argc, char **argv) {
	if(argc != 2) {
		cerr << "predict_ensenmble config." << endl;
		exit(1);
	}
	
	string	path_config = argv[1];
	const Config	config(path_config);
	Ensemble::process(config);
}
