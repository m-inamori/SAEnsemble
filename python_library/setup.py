from distutils.core import setup, Extension

sources = ['predict.cpp', 'Predictor.cpp', 'common.cpp',
			'SimulatedAnnealing.cpp', 'statistics.cpp', 'config.cpp' ]
setup(name = 'SAEnsemble', version = '1.0.0', \
	ext_modules = [Extension('SAEnsemble', sources)])
