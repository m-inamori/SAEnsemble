// predict.cpp
// Interface between Python and C++

#include <iostream>
#include <map>
#include <Python.h>
#include "Predictor.h"

using namespace Statistics;

static void py_free(PyObject *obj) {
	
}

static Vec make_vector(PyObject *obj_list) {
	if(!PyList_Check(obj_list))
		return Vec(0);
	
	Vec	v;
	const int	M = PyList_Size(obj_list);
	for(int j = 0; j < M; ++j) {
		PyObject	*obj = PyList_GetItem(obj_list, (Py_ssize_t)j);
		if(!PyFloat_Check(obj)) {
			printf("vector[%d] is not float.\n", j);
			return Vec(0);
		}
		
		const double	d = PyFloat_AsDouble(obj);
		v.push_back(d);
	}
	
	return v;
}

static bool is_same_row_length(const Matrix& M) {
	if(M.empty())
		return false;
	
	const size_t	L = M.front().size();
	for(auto p = M.begin(); p != M.end(); ++p) {
		if(p->size() != L) {
			std::cerr << "The matrix is not the same row length." << std::endl;
			return false;
		}
	}
	return true;
}

static Matrix make_matrix(PyObject *obj_mat) {
	if(!PyList_Check(obj_mat))
		return Matrix(0);
	
	const int	L = PyList_Size(obj_mat);
	Matrix	X(L);
	for(int i = 0; i < L; ++i) {
		PyObject	*o = PyList_GetItem(obj_mat, (Py_ssize_t)i);
		Vec	v = make_vector(o);
		if(v.empty())
			return Matrix(0);
		X[i] = v;
	}
	
	if(!is_same_row_length(X))
		return Matrix(0);
	
	return X;
}

static PyObject *Vec_to_List(const Vec& v) {
	PyObject	*list = PyList_New((int)v.size());
	for(int i = 0; i < (int)v.size(); ++i)
		PyList_SET_ITEM(list, i, PyFloat_FromDouble(v[i]));
	return list;
}

static PyObject *VecInt_to_List(const std::vector<int>& v) {
	PyObject	*list = PyList_New((int)v.size());
	for(int i = 0; i < (int)v.size(); ++i) {
		if(i != 2) {
			PyList_SET_ITEM(list, i, PyLong_FromLong(v[i]));
		}
		else {		// "And" or "Or"
			if(v[i] == 1)
				PyList_SET_ITEM(list, i, Py_BuildValue("s", "And"));
			else
				PyList_SET_ITEM(list, i, Py_BuildValue("s", "Or"));
		}
	}
	return list;
}

static PyObject *make_model(PyObject *self, PyObject *args, PyObject *kwdict) {
	PyObject	*obj_mat;
	PyObject	*obj_vec;
	int			num_main		= 10;
	int			num_second		= 10;
	int			max_time		= 5000;
	int			max_interval	= 500;
	int			num_iterations	= 1000;
	int			seed			= 2;
	int			num_threads		= 1;
	static char	*keywords[] = {
		"X", "y", "num_main", "num_second", "max_time", "max_interval",
		"num_iterations", "seed", "num_threads", NULL
	};
	if(!PyArg_ParseTupleAndKeywords(
			args, kwdict, "OO|iiiiiii", keywords, &obj_mat, &obj_vec,
			&num_main, &num_second, &max_time, &max_interval,
			&num_iterations, &seed, &num_threads))
		return NULL;
	
	Matrix	X = make_matrix(obj_mat);
	if(X.empty())
		return NULL;
	
	Vec	y = make_vector(obj_vec);
	if(y.empty())
		return NULL;
	
	auto	*model = Predictor::make_model(X, y,
							num_main, num_second, max_time, max_interval,
							num_iterations, seed, num_threads);
	if(model == NULL)
		return NULL;
	
	return PyCapsule_New(model, "_predict", py_free);
}

static PyObject *predict(PyObject *self, PyObject *args, PyObject *kwdict) {
	PyObject	*obj_model;
	PyObject	*obj_X;
	const char	*weight_type = NULL;
	int			num_learners = 0;
	double		weight_exp = 0.0;
	static char	*keywords[] = {
		"model", "X", "weight_type", "num_learners", "weight_exp", NULL
	};
	if (!PyArg_ParseTupleAndKeywords(args, kwdict, "OO|sid", keywords,
										&obj_model, &obj_X, &weight_type,
										&num_learners, &weight_exp))
		return NULL;
	
	Predictor	*model = (Predictor *)PyCapsule_GetPointer(
											obj_model, "_predict");
	Matrix	X = make_matrix(obj_X);
	if(X.empty())
		return NULL;
	
	Vec	p = model->predict(X, weight_type, num_learners, weight_exp);
	PyObject	*list = Vec_to_List(p);
	return list;
}

static PyObject *correlations(PyObject *self, PyObject *args) {
	PyObject	*obj_model;
	if (!PyArg_ParseTuple(args, "O", &obj_model))
		return NULL;
	
	Predictor	*model = (Predictor *)PyCapsule_GetPointer(
											obj_model, "_predict");
	Vec	corrs = model->correlations();
	PyObject	*list = Vec_to_List(corrs);
	return list;
}

static PyObject *PyList_FromVariables(const Variables& vars) {
	PyObject	*list = PyList_New((int)vars.size());
	for(int i = 0; i < (int)vars.size(); ++i) {
		PyList_SET_ITEM(list, i, VecInt_to_List(vars.get_vars(i)));
	}
	return list;
}

static PyObject *variables(PyObject *self, PyObject *args) {
	PyObject	*obj_model;
	if (!PyArg_ParseTuple(args, "O", &obj_model))
		return NULL;
	
	Predictor	*model = (Predictor *)PyCapsule_GetPointer(
											obj_model, "_predict");
	const std::vector<const Variables *> var_table = model->variables();
	
	PyObject	*list = PyList_New((int)var_table.size());
	for(int i = 0; i < (int)var_table.size(); ++i) {
		PyList_SET_ITEM(list, i, PyList_FromVariables(*(var_table[i])));
	}
	return list;
}

static PyMethodDef predMethods[] = {
	{ "make_model",   (PyCFunction)make_model, METH_VARARGS | METH_KEYWORDS },
	{ "predict",      (PyCFunction)predict,    METH_VARARGS | METH_KEYWORDS },
	{ "correlations", correlations,            METH_VARARGS },
	{ "variables",    variables,               METH_VARARGS },
	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef SAEnsemble = {
	PyModuleDef_HEAD_INIT, "SAEnsemble", "Python3 C API Module(Sample 2)",
	-1, predMethods
};

PyMODINIT_FUNC PyInit_SAEnsemble() {
	return PyModule_Create(&SAEnsemble);
}
