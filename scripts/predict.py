# coding: utf-8
# predict.py
# SAの結果の上位でアンサンブル予測する
# C++によるLibraryを使う

from __future__ import annotations
from itertools import *
from typing import Iterator, Optional
import csv
import sys

import SAEnsemble
from config import *
from common import read_csv


#################### Config ####################

class Config(ConfigCheckable):
	def __init__(self, path):
		super().__init__(path)
		b1 = super().check_existence('GENOTYPE', 'PHENOTYPE', 'TRAIT', 'OUTPUT')
		b2 = super().check_existence_int('NUM_MAIN_EFFECT_VARIABLES',
										 'NUM_SECOND_CAUSAL_VARIABLES',
										 'MAX_TIME', 'NUM_ITERATIONS',
										 'MAX_INVARIANT_TIME', 'NUM_LEARNERS',
										 'NUM_THREADS')
		b3 = super().check_int('RANDOM_SEED')
		
		if not all((b1, b2, b3)):
			exit(1)
	
	def genotype(self):			return self.dic['GENOTYPE']
	def phenotype(self):		return self.dic['PHENOTYPE']
	def trait(self):			return self.dic['TRAIT']
	def path_CV(self):			return self.dic['PATH_CV']
	def out(self):				return self.dic['OUTPUT']
	def max_time(self):			return int(self.dic['MAX_TIME'])
	def max_invariant(self):	return int(self.dic['MAX_INVARIANT_TIME'])
	def num_iterations(self):	return int(self.dic['NUM_ITERATIONS'])
	def num_learners(self):		return int(self.dic['NUM_LEARNERS'])
	def num_divisions(self):	return int(self.dic['NUM_DIVISIONS'])
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

def predict(X: list[list[float]], y: list[float], config: Config):
	model = SAEnsemble.make_model(
						X=X,
						y=y,
						num_main=config.num_main_effect_variables(),
						num_second=config.num_second_causal_variables(),
						max_time=config.max_time(),
						max_interval=config.max_invariant(),
						num_iterations=config.num_iterations(),
						seed=config.seed(),
						num_threads=config.num_threads())
	
	p = SAEnsemble.predict(model=model, X=X,
								weight_type=config.weight_type(),
								num_learners=config.num_learners(),
								weight_exp=config.weight_exponent())
	return p

def write_results(p: list[float], config: Config):
	samples, y = get_pheno(config)
	
	with open(config.out(), 'w') as out:
		writer = csv.writer(out)
		trait = config.trait()
		writer.writerow(['sample', 'y', 'p'])
		for s, y1, p1 in zip(samples, y, p):
			writer.writerow([s, y1, p1])


#################### main ####################

if len(sys.argv) != 2:
	print("usage : predict.py config.", file=sys.stderr)
	exit(1)

config = Config(sys.argv[1])
X = list(read_geno(config.genotype()))
_, y = get_pheno(config)
p = predict(X, y, config)
write_results(p, config)
