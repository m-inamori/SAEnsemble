// statistics.h

#ifndef INCLUDE_STATISTICS
#define	INCLUDE_STATISTICS

#include <vector>
#include <numeric>

namespace Statistics {
	static const double	eps = 1e-8;
	
	typedef std::vector<double>	Vec;
	typedef std::vector<Vec>	Matrix;
	
	Matrix inverse(const Matrix& A);
	Matrix unit_matrix(size_t L);
	Matrix multiply_transpose_and_normal(const Matrix& X);
	Vec sub(const Vec& v, const Vec& w);
	double inner_product(const Vec& v, const Vec& w);
	Matrix multiply(const Matrix& A, const Matrix& B);
	Vec multiply_Mat_and_Vec(const Matrix& X, const Vec& y);
	Vec multiply_Vec_and_Mat(const Vec& y, const Matrix& X);
	Matrix transpose(const Matrix& X);
	Matrix LU_decompose(const Matrix& X);
	Vec multiply_U_Inv_Vec(const Matrix& LU, const Vec& v);
	Vec multiply_L_Inv_Vec(const Matrix& LU, const Vec& v);
	
	void print(const Matrix& A);
	void print(const Vec& v);
	
	class RegressionResult {
	public:
		std::vector<double>	pred;
		std::vector<double>	b;
		
	public:
		RegressionResult(const Vec& p_, const Vec& b_) : pred(p_), b(b_) { }
	};
	
	double average(const Vec& v);
	double corr(const Vec& x, const Vec& y);
	RegressionResult multiple_regression(const Matrix& X, const Vec& y);
	RegressionResult multiple_regression_core(
					const Matrix& XX, const Matrix& X, const Vec& y);
}

#endif
