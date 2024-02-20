// CV.h

#include <vector>
#include "random.h"

class CrossValidation {
	const std::vector<std::string>	samples;
	const std::vector<std::vector<int>>	table;
	
public:
	CrossValidation(const std::vector<std::string>& s,
					const std::vector<std::vector<int>>& t) :
											samples(s), table(t) { }
	
	std::size_t num_repeats() const {
		return table.size();
	}
	
	int num_divisions() {
		int	max_group = 0;
		for(auto p = table.begin(); p != table.end(); ++p) {
			if(p->front() > max_group)
				max_group = p->front();
		}
		return max_group;
	}
	
	std::vector<std::vector<double>> extract_X_train(
									const std::vector<std::vector<double>>& X,
									std::size_t k, int g);
	std::vector<double> extract_y_train(const std::vector<double>& y,
										std::size_t k, int g);
	std::vector<std::vector<double>> extract_X_test(
									const std::vector<std::vector<double>>& X,
									std::size_t k, int g);
	
	void set_values(std::vector<double>& pred, const std::vector<double>& p_sub,
									std::size_t k, int g);
	void write(const std::string& path) const;
	
	static CrossValidation read(const std::string& path);
};
