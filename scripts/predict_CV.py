# coding: utf-8
# predict.py
# SAの結果の上位でアンサンブル予測する
# C++によるLibraryを使う

from __future__ import annotations
from itertools import *
from typing import Iterator, Optional, Any
import csv
import sys
import time

import SAEnsemble
from CV import CrossValidation
from config import *
from common import read_csv, write_csv


#################### Config ####################

class Config(ConfigCheckable):
	def __init__(self, path):
		super().__init__(path)
		b1 = super().check_existence('GENOTYPE', 'PHENOTYPE',
										'TRAIT', 'PATH_CV', 'OUTPUT')
		b2 = super().check_existence_int('NUM_MAIN_EFFECT_VARIABLES',
										 'NUM_SECOND_CAUSAL_VARIABLES',
										 'MAX_TIME', 'NUM_ITERATIONS',
										 'MAX_INVARIANT_TIME', 'NUM_LEARNERS',
										 'NUM_THREADS')
		b3 = super().check_int('RANDOM_SEED')
		b4 = self.__is_valid_weight()
		
		if not all((b1, b2, b3, b4)):
			exit(1)
	
	def __is_valid_weight(self):
		if 'WEIGHT_TYPE' not in self.dic:
			return True		# rsq
		elif (self.weight_type() == 'truncated' or
						self.weight_type() == 'linear'):
			return self.__is_valid_num_learners()
		elif self.weight_type() == 'power':
			return (super().check_existence('WEIGHT_EXP') and
					super().check_float('WEIGHT_EXP'))
		elif self.weight_type() == 'rsq':
			return True
		else:
			print("error : invalid WEIGHT_TYPE %s." % self.weight_type(),
														file=sys.stderr)
			return False
	
	def __is_valid_num_learners(self):
		if not super().check_existence_int('NUM_LEARNERS'):
			return False
		elif self.num_learners() > self.num_iterations():
			print("error : NUM_LEARNERS must be less than", file=sys.stderr)
			print("or equal to NUM_ITERATIONS.", file=sys.stderr)
			return False
		elif self.num_learners() <= 0:
			print("error : NUM_LEARNERS must be positive.", file=sys.stderr)
			return False
		else:
			return True
	
	def genotype(self):			return self.dic['GENOTYPE']
	def phenotype(self):		return self.dic['PHENOTYPE']
	def trait(self):			return self.dic['TRAIT']
	def path_CV(self):			return self.dic['PATH_CV']
	def out(self):				return self.dic['OUTPUT']
	def max_time(self):			return int(self.dic['MAX_TIME'])
	def max_invariant(self):	return int(self.dic['MAX_INVARIANT_TIME'])
	def num_iterations(self):	return int(self.dic['NUM_ITERATIONS'])
	def num_learners(self):		return int(self.dic.get('NUM_LEARNERS', '0'))
	def num_repeats(self):		return int(self.dic['NUM_REPEATS'])
	def num_threads(self):		return int(self.dic['NUM_THREADS'])
	def seed(self):				return int(self.dic.get('RANDOM_SEED', 2))
	def weight_type(self):		return self.dic.get('WEIGHT_TYPE', 'rsq')
	def weight_exponent(self):	return float(self.dic.get('WEIGHT_EXP', '1.0'))
	
	def num_main_effect_variables(self):
		return int(self.dic['NUM_MAIN_EFFECT_VARIABLES'])
	
	def num_second_causal_variables(self):
		return int(self.dic['NUM_SECOND_CAUSAL_VARIABLES'])


#################### process ####################

def read_geno(path: str) -> Iterator[list[float]]:
	for v in islice(read_csv(path), 1, None):
		yield [ float(e) for e in v[1:] ]

def read_pheno(path: str) -> tuple[list[str], list[str], dict[str, list[str]]]:
	table = list(read_csv(path))
	traits = table[0][1:]
	samples = [ v[0] for v in table[1:] ]
	pheno = { traits[i]: [ v[i+1] for v in table[1:] ]
									for i in range(len(traits)) }
	return (traits, samples, pheno)

def get_pheno(config: Config) -> tuple[list[str], list[float]]:
	# [trait], { trait: [pheno] }
	path_pheno = config.phenotype()
	traits, samples, pheno = read_pheno(path_pheno)
	trait = config.trait()
	if trait not in traits:
		print("%s is not found in %s." % (trait, path_pheno), file=sys.stderr)
		exit(1)
	
	return (samples, list(map(float, pheno[trait])))

def predict_each(X: list[list[float]], y: list[float], k: int,
							cv: CrossValidation, config: Config) -> list[float]:
	p = [0.0] * len(y)
	for g in range(1, cv.num_divisions()+1):
		model = SAEnsemble.make_model(
						X=cv.extract_X_train(X, k, g),
						y=cv.extract_y_train(y, k, g),
						num_main=config.num_main_effect_variables(),
						num_second=config.num_second_causal_variables(),
						max_time=config.max_time(),
						max_interval=config.max_invariant(),
						num_iterations=config.num_iterations(),
						seed=config.seed(),
						num_threads=config.num_threads())
		
		p_sub = SAEnsemble.predict(model=model,
								X=cv.extract_X_test(X, k, g),
#								X=X,
								weight_type=config.weight_type(),
								num_learners=config.num_learners(),
								weight_exp=config.weight_exponent())
		cv.set_values(p, p_sub, k, g)
		print("%d %d %.3fs" % (k+1, g, time.time() - t0))
	return p

def write_results(ps: list[list[float]], config: Config):
	samples, y = get_pheno(config)
	
	table: list[list[Any]] = [ [''] * (len(ps)+2) for _ in range(len(y)+1) ]
	table[0][0] = 'sample'
	table[0][1] = 'y'
	for j in range(len(ps)):
		table[0][j+2] = 'p%d' % (j+1)
	for i in range(len(y)):
		table[i+1][0] = samples[i]
		table[i+1][1] = y[i]
		for j in range(len(ps)):
			table[i+1][j+2] = ps[j][i]
	write_csv(table, config.out())

def predict_CV(config: Config):
	X = list(read_geno(config.genotype()))
	_, y = get_pheno(config)
	cv = CrossValidation.read(config.path_CV())
	ps = [ predict_each(X, y, k, cv, config)
						for k in range(cv.num_repeats()) ]
	write_results(ps, config)


#################### main ####################

if len(sys.argv) != 2:
	print("usage : predict.py config.", file=sys.stderr)
	exit(1)

t0 = time.time()
config = Config(sys.argv[1])
predict_CV(config)
