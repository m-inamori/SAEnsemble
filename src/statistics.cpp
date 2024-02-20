// statistics.cpp

#include <iostream>
#include <cmath>
#include <stdexcept>
#include "../include/statistics.h"

using namespace std;
using namespace Statistics;

Vec Statistics::sub(const Vec& v, const Vec& w) {
	Vec	x(v.size());
	for(size_t i = 0; i < v.size(); ++i)
		x[i] = v[i] - w[i];
	return x;
}

double Statistics::inner_product(const Vec& v, const Vec& w) {
	double	s = 0.0;
	for(size_t i = 0; i < v.size(); ++i)
		s += v[i] * w[i];
	return s;
}

double Statistics::corr(const Vec& x, const Vec& y) {
	const double	mx = average(x);
	const double	my = average(y);
	
	double	sum_xx = 0.0;
	double	sum_yy = 0.0;
	double	sum_xy = 0.0;
	for(int k = 0; k < (int)x.size(); ++k) {
		sum_xx += (x[k] - mx) * (x[k] - mx);
		sum_yy += (y[k] - my) * (y[k] - my);
		sum_xy += (x[k] - mx) * (y[k] - my);
	}
	
	return sum_xy / sqrt(sum_xx * sum_yy);
}

void Statistics::print(const Matrix& A) {
	for(auto p = A.begin(); p != A.end(); ++p) {
		const auto&	v = *p;
		for(auto q = v.begin(); q != v.end(); ++q) {
			cout << *q << " ";
		}
		cout << endl;
	}
}

void Statistics::print(const Vec& v) {
	for(auto q = v.begin(); q != v.end(); ++q) {
		cout << *q << " ";
	}
	cout << endl;
}

Matrix Statistics::inverse(const Matrix& A_) {
	// Gauss-Seidel
	Matrix	A = A_;
	const size_t	L = A.size();
	Matrix	B = unit_matrix(L);
	for(size_t i = 0U; i < L; ++i) {
		pair<double,size_t>	max_abs = make_pair(0.0, (size_t)0U);
		for(size_t j = i; j < L; ++j) {
			max_abs = max(max_abs, make_pair(fabs(A[j][i]), j));
		}
		const size_t	j0 = max_abs.second;
		const double	a0 = A[j0][i];
		if(fabs(a0) < eps) {
			throw(overflow_error("singular matrix"));
		}
		for(size_t k = i; k < L; ++k) {
			A[j0][k] /= a0;
		}
		for(size_t k = 0; k < L; ++k) {
			B[j0][k] /= a0;
		}
		for(size_t j = i; j < L; ++j) {
			if(j == j0)
				continue;
			const double	a1 = A[j][i];
			for(size_t k = i; k < L; ++k) {
				A[j][k] -= a1 * A[j0][k];
			}
			for(size_t k = 0; k < L; ++k) {
				B[j][k] -= a1 * B[j0][k];
			}
		}
		if(j0 != i) {
			for(size_t k = i; k < L; ++k) {
				A[i][k] += A[j0][k];
			}
			for(size_t k = 0; k < L; ++k) {
				B[i][k] += B[j0][k];
			}
			for(size_t k = i; k < L; ++k) {
				A[j0][k] -= A[i][k];
			}
			for(size_t k = 0; k < L; ++k) {
				B[j0][k] -= B[i][k];
			}
		}
	}
	
	for(size_t i = L-1; i > 0U; --i) {
		for(size_t j = 0; j < i; ++j) {
			const double	a1 = A[j][i];
			A[j][i] -= a1 * A[i][i];
			for(size_t k = 0; k < L; ++k) {
				B[j][k] -= a1 * B[i][k];
			}
		}
	}
	
	return B;
}

Matrix Statistics::unit_matrix(size_t L) {
	Matrix	I(L, Vec(L, 0.0));
	for(size_t k = 0; k < L; ++k)
		I[k][k] = 1.0;
	return I;
}

Matrix Statistics::multiply(const Matrix& A, const Matrix& B) {
	const size_t	H = A.size();
	const size_t	M = B.size();
	const size_t	W = B.front().size();
	
	const Matrix	T = transpose(B);
	Matrix	C(H, Vec(W, 0.0));
	for(size_t i = 0U; i < H; ++i) {
		for(size_t j = 0U; j < W; ++j) {
			for(size_t k = 0U; k < M; ++k)
				C[i][j] += A[i][k] * T[j][k];
		}
	}
	return C;
}

Vec Statistics::multiply_Mat_and_Vec(const Matrix& X, const Vec& y) {
	const size_t	H = X.size();
	const size_t	L = y.size();
	Vec	v(H);
	for(size_t k = 0U; k < H; ++k) {
		double	s = 0.0;
		for(size_t i = 0U; i < L; ++i)
			s += X[k][i] * y[i];
		v[k] = s;
	}
	return v;
}

Vec Statistics::multiply_Vec_and_Mat(const Vec& y, const Matrix& X) {
	const size_t	H = X.size();
	const size_t	L = X.front().size();
	Vec	v(L);
	for(size_t k = 0U; k < L; ++k) {
		double	s = 0.0;
		for(size_t i = 0U; i < H; ++i)
			s += y[i] * X[i][k];
		v[k] = s;
	}
	return v;
}

Matrix Statistics::multiply_transpose_and_normal(const Matrix& X) {
	const size_t	H = X.size();
	const size_t	L = X.front().size();
	Matrix	XX(H, Vec(H));
	for(size_t i = 0U; i < H; ++i) {
		for(size_t j = i; j < H; ++j) {
			double	e = 0.0;
			for(size_t k = 0; k < L; ++k)
				e += X[i][k] * X[j][k];
			XX[i][j] = e;
		}
	}
	
	for(size_t i = 0U; i < H; ++i) {
		for(size_t j = 0U; j < i; ++j)
			XX[i][j] = XX[j][i];
	}
	
	return XX;
}

Matrix Statistics::transpose(const Matrix& X) {
	const size_t	H = X.size();
	const size_t	L = X.front().size();
	Matrix	T(L, Vec(H));
	for(size_t i = 0U; i < H; ++i) {
		for(size_t j = 0U; j < L; ++j)
			T[j][i] = X[i][j];
	}
	return T;
}

// https://en.wikipedia.org/wiki/LU_decomposition
Matrix Statistics::LU_decompose(const Matrix& X) {
	const size_t	L = X.size();
	if(X.front().size() != L) {
		throw(overflow_error("not square matrix"));
	}
	
	// LU is a matrix which is joined L and U
	// LU includes the diagonal of U (not that of L)
	Matrix	LU(L, Vec(L, 0.0));
	for(int i = L-1; i >= 0 ; --i) {
		// upper
		for(int j = L-1; j >= i; --j) {
			LU[i][j] = X[i][j];
			for(int k = L-1; k > j; --k)
				LU[i][j] -= LU[i][k] * LU[k][j];
		}
		if(fabs(LU[i][i]) < eps)
			throw(overflow_error("singular matrix"));
		
		// lower
		for(int j = i-1; j >= 0; --j) {
			LU[i][j] = X[i][j];
			for(int k = i+1; k < (int)L; ++k)
				LU[i][j] -= LU[i][k] * LU[k][j];
			LU[i][j] /= LU[i][i];
		}
	}
	
	return LU;
}

Vec Statistics::multiply_U_Inv_Vec(const Matrix& LU, const Vec& v) {
	const size_t	N = v.size();
	Vec	w(N, 0.0);
	for(size_t i = N-1U; ; --i) {
		w[i] = v[i];
		for(size_t j = i+1U; j < N; ++j)
			w[i] -= LU[i][j] * w[j];
		w[i] /= LU[i][i];
		if(i == 0U)
			break;
	}
	return w;
}

Vec Statistics::multiply_L_Inv_Vec(const Matrix& LU, const Vec& v) {
	const size_t	N = v.size();
	Vec	w(N, 0.0);
	for(size_t i = 0U; i < N; ++i) {
		w[i] = v[i];
		for(size_t j = 0U; j < i; ++j)
			w[i] -= LU[i][j] * w[j];
	}
	return w;
}

RegressionResult Statistics::multiple_regression(const Matrix& X,
													const Vec& y) {
	const Matrix	XX = multiply_transpose_and_normal(X);	// X'X
	return multiple_regression_core(XX, X, y);
}

RegressionResult Statistics::multiple_regression_core(
						const Matrix& XX, const Matrix& X, const Vec& y) {
	const Matrix	LU = LU_decompose(XX);
	const Vec	v1 = multiply_Mat_and_Vec(X, y);
	const Vec	v2 = multiply_U_Inv_Vec(LU, v1);	// U^(-1)Xy
	const Vec	b = multiply_L_Inv_Vec(LU, v2);		// L^(-1)U^(-1)Xy
	const Vec	y_pred = multiply_Vec_and_Mat(b, X);
	return RegressionResult(y_pred, b);
}

double Statistics::average(const Vec& v) {
	return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
}

#if 0
int main(int argc, char **argv) {
	Vec	xs { 0.0, 0.0, 1.0, 1.0 };
	Vec	ys { 0.0, 1.0, 1.0, 0.0 };
	Vec	zs { 2.0, 3.0, 7.0, 2.0 };
	
	/*
	double	sum_n = 4.0;
	double	sum_x = 0.0;
	double	sum_y = 0.0;
	double	sum_z = 0.0;
	double	sum_xx = 0.0;
	double	sum_xy = 0.0;
	double	sum_xz = 0.0;
	double	sum_yy = 0.0;
	double	sum_yz = 0.0;
	for(int i = 0; i < 4; ++i) {
		sum_x += xs[i];
		sum_y += ys[i];
		sum_z += zs[i];
		sum_xx += xs[i] * xs[i];
		sum_xy += xs[i] * ys[i];
		sum_xz += xs[i] * zs[i];
		sum_yy += ys[i] * ys[i];
		sum_yz += ys[i] * zs[i];
	}
	
	Matrix	X(3, Vec(3, 0.0));
	X[0][0] = 4.0;   X[0][1] = sum_x;  X[0][2] = sum_y;
	X[1][0] = sum_x; X[1][1] = sum_xx; X[1][2] = sum_xy;
	X[2][0] = sum_y; X[2][1] = sum_xy; X[2][2] = sum_yy;
	
	Vec	v(3);
	v[0] = sum_z;
	v[1] = sum_xz;
	v[2] = sum_yz;
	*/
	
	Matrix	X(3, Vec(4, 1.0));
	for(int i = 0; i < 4; ++i) {
		X[0][i] = 1.0;
		X[1][i] = xs[i];
		X[2][i] = ys[i];
	}
	
	auto	result = multiple_regression(X, zs);
	print(result.b);
	print(result.pred);
}
#endif