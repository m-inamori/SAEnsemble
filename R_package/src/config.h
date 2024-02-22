// config.h

#ifndef __CONFIG
#define __CONFIG


//////////////////// Config ////////////////////

class Config {
	const int		NUM_MAIN_EFFECT_VARIABLES;
	const int		NUM_SECOND_CAUSAL_VARIABLES;
	const int		MAX_TIME;
	const int		MAX_INVARIANT_TIME;
	const int		NUM_ITERATIONS;
	const int		SEED;
	const int		NUM_THREADS;
	
public:
	Config(int num_main, int num_second, int max_t, int max_int_time,
								int num_iters, int s, int num_threads_) :
				NUM_MAIN_EFFECT_VARIABLES(num_main),
				NUM_SECOND_CAUSAL_VARIABLES(num_second),
				MAX_TIME(max_t), MAX_INVARIANT_TIME(max_int_time),
				NUM_ITERATIONS(num_iters), SEED(s),
				NUM_THREADS(num_threads_) { }
	
	bool is_valid() const;
	
	int num_main_vars() const		{ return NUM_MAIN_EFFECT_VARIABLES; }
	int num_second_vars() const		{ return NUM_SECOND_CAUSAL_VARIABLES; }
	int max_time() const			{ return MAX_TIME; }
	int max_invariant() const		{ return MAX_INVARIANT_TIME; }
	int num_iterations() const		{ return NUM_ITERATIONS; }
	int num_threads() const			{ return NUM_THREADS; }
	int seed() const				{ return SEED; }
};

#endif
