# coding: utf-8
# rsq2.py
# rsq.pyがメモリをやけに食うので、コードを変える

from itertools import *
import numpy as np
import csv
import sys
import time

import SAEnsemble


#################### library ####################

def read_csv(path):
	with open(path, 'r') as f:
		reader = csv.reader(f)
		for row in reader:
			yield row

def read_tsv(path):
	with open(path, 'r') as f:
		reader = csv.reader(f, delimiter='\t')
		for row in reader:
			yield row

def write_csv(v, out):
	print(','.join(map(str, v)), file=out)

def average(iterable):
	total = 0.0
	counter = 0
	for e in iterable:
		total += e
		counter += 1
	return total / counter

def stdev(iterable):
	v = [ e for e in iterable ]
	mean = average(v)
	return sqrt(average((e-mean)**2 for e in v))

def calc_corr(ys1, ys2):
	pairs = [ (y1, y2) for y1, y2 in zip(ys1, ys2)
						if y1 is not None and y2 is not None ]
	v = list(zip(*pairs))
	return np.corrcoef(np.array(v[0]), np.array(v[1]))[0, 1]

def floatNA(s):
	try:
		return float(s)
	except ValueError:
		return None


#################### ConfigBase ####################

class ConfigBase:
	def __init__(self, path):
		self.dic = { }
		with open(path, 'r') as f:
			for line in f:
				if line.startswith('#'):
					continue
				elif not line.strip():	# empty line
					continue
				elif line.count('=') != 1:
					raise Exception("format error : %s" % line.rstrip())
				
				[key, value] = [ s.strip() for s in line.split('=') ]
				if not key or not value:
					raise Exception("format error : %s" % line.rstrip())
				self.dic[key] = value
	
	def exists_key(self, key):
		return key in self.dic
	
	def get(self, key):
		return self.dic.get(key)


#################### ConfigCheckable ####################

class ConfigCheckable(ConfigBase):
	def __init__(self, path):
		super().__init__(path)
	
	def check_existence(self, *keys):
		OK = True
		for key in keys:
			if key not in self.dic:
				OK = False
				print("key %s must exist." % key, file=sys.stderr)
		return OK
	
	def __check_existence(self, keys, checker, error_message):
		def is_valid(key):
			if key not in self.dic:
				print("key %s must exist." % key, file=sys.stderr)
				return False
			elif not checker(self.dic[key]):
				print(error_message % key, file=sys.stderr)
				return False
			else:
				return True
		
		bs = list(map(is_valid, keys))
		return all(bs)
	
	def __check_type_if_exists(self, keys, checker, error_message):
		def is_valid(key):
			if key in self.dic and not checker(self.dic[key]):
				print(error_message % key, file=sys.stderr)
				return False
			else:
				return True
		
		bs = list(map(is_valid, keys))
		return all(bs)
	
	def check_existence_int(self, *keys):
		return self.__check_existence(keys, ConfigCheckable.is_int,
											"key %s must be integer.")
	
	def check_existence_float(self, *keys):
		return self.__check_existence(keys, ConfigCheckable.is_float,
											"key %s must be float.")
	
	def check_existence_bool(self, *keys):
		return self.__check_existence(keys, ConfigCheckable.is_bool,
											"key %s must be boolean.")
	
	def check_int(self, *keys):
		return self.__check_type_if_exists(keys, ConfigCheckable.is_int,
											"key %s must be integer.")
	
	def check_bool(self, *keys):
		return self.__check_type_if_exists(keys, ConfigCheckable.is_bool,
											"key %s must be boolean.")
	
	def check_float(self, *keys):
		return self.__check_type_if_exists(keys, ConfigCheckable.is_float,
											"key %s must be float number.")
	
	@staticmethod
	def is_int(s):
		try:
			int(s)
			return True
		except ValueError:
			return False
	
	@staticmethod
	def is_bool(s):
		try:
			bool(s)
			return True
		except ValueError:
			return False
	
	@staticmethod
	def is_float(s):
		try:
			float(s)
			return True
		except ValueError:
			return False


#################### Config ####################

class Config(ConfigCheckable):
	def __init__(self, path):
		super().__init__(path)
		b1 = super().check_existence('GENOTYPE', 'PHENOTYPE',
									 'TRAIT', 'PATH_CV',
									 'OUTPUT', 'WEIGHT_TYPE')
		b2 = super().check_existence_int('NUM_MAIN_EFFECT_VARIABLES',
										 'NUM_SECOND_CAUSAL_VARIABLES',
										 'MAX_TIME', 'NUM_ITERATIONS',
										 'MAX_INVARIANT_TIME',
										 'NUM_THREADS')
		b3 = super().check_int('RANDOM_SEED')
		b4 = self.__is_valid_weight()
		
		if not all((b1, b2, b3, b4)):
			exit(1)
	
	def __is_valid_weight(self):
		if 'WEIGHT_TYPE' not in self.dic:
			return False
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
	def weight_type(self):		return self.dic['WEIGHT_TYPE']
	def weight_exponent(self):	return float(self.dic.get('WEIGHT_EXP', '0'))
	def num_learners(self):		return int(self.dic.get('NUM_LEARNERS', '0'))
	def num_divisions(self):	return int(self.dic['NUM_DIVISIONS'])
	def num_repeats(self):		return int(self.dic['NUM_REPEATS'])
	def num_threads(self):		return int(self.dic['NUM_THREADS'])
	def seed(self):				return int(self.dic.get('RANDOM_SEED', 2))
	
	def num_main_effect_variables(self):
		return int(self.dic['NUM_MAIN_EFFECT_VARIABLES'])
	
	def num_second_causal_variables(self):
		return int(self.dic['NUM_SECOND_CAUSAL_VARIABLES'])


#################### CrossValidation ####################

class CrossValidation:
	def __init__(self, path_CV, y):
		self.CV_table = self.read_CV(path_CV)
		self.y = y
	
	def read_CV(self, path):
		table = [ [ int(e)-1 for e in v[1:] ]
					for v in islice(read_csv(path), 1, None) ]
		return list(zip(*table))	# transpose
	
	def num_repeats(self):
		return len(self.CV_table)
	
	def num_divisions(self):
		return max(self.CV_table[0]) + 1
	
	def make_X_train(self, X, k, g):
		v = self.CV_table[k]
		return [ [ row[i] for i, g1 in enumerate(v)
						if self.y[i] is not None and g1 != g ] for row in X ]
	
	def make_y_train(self, y, k, g):
		v = self.CV_table[k]
		return [ y[i] for i, g1 in enumerate(v)
						if self.y[i] is not None and g1 != g ]
	
	def make_X_test(self, X, k, g):
		v = self.CV_table[k]
		return [ [ row[i] for i, g1 in enumerate(v)
						if self.y[i] is not None and g1 == g ] for row in X ]
	
	def set_values(self, pred, p_sub, k, g):
		v = self.CV_table[k]
		for i, value in zip((i for i, g1 in enumerate(v)
						if self.y[i] is not None and g1 == g), p_sub):
			pred[i] = value


#################### process ####################

def read_geno(path):
	for v in islice(read_csv(path), 1, None):
		yield [ float(e) for e in v[1:] ]

def read_pheno(path):
	table = list(read_csv(path))
	traits = table[0][1:]
	samples = [ v[0] for v in table[1:] ]
	pheno = dict((traits[i], [ v[i+1] for v in table[1:] ])
								for i in range(len(traits)))
	return (traits, samples, pheno)

def get_pheno(config):
	# [trait], { trait: [pheno] }
	path_pheno = config.phenotype()
	traits, samples, pheno = read_pheno(path_pheno)
	trait = config.trait()
	if trait not in traits:
		print("%s is not found in %s." % (trait, path_pheno), file=sys.stderr)
		exit(1)
	
	return (samples, list(map(floatNA, pheno[trait])))

def optimize(cv, config):
	def make_models():
		model_table = []
		return model_table
	
	def div(n, d):
		if isinstance(n, int):
			return n // d
		else:
			return n / d
	
	X = list(read_geno(config.genotype()))
	y = cv.y
	ps = []
	for k in range(cv.num_repeats()):
		pred = [ None ] * len(y)
		for g in range(cv.num_divisions()):
			seed = (config.num_iterations() * (cv.num_repeats() * k + g)
												+ config.seed())
			model = SAEnsemble.make_model(
						X=cv.make_X_train(X, k, g),
						y=cv.make_y_train(y, k, g),
						num_main=config.num_main_effect_variables(),
						num_second=config.num_second_causal_variables(),
						max_time=config.max_time(),
						max_interval=config.max_invariant(),
						num_iterations=config.num_iterations(),
						seed=seed,
						num_threads=config.num_threads())
			X_test = cv.make_X_test(X, k, g)
			p_sub = SAEnsemble.predict(model=model, X=X_test,
										weight_type=config.weight_type(),
										num_learners=config.num_learners(),
										weight_exp=config.weight_exponent())
			cv.set_values(pred, p_sub, k, g)
			print(k+1, g+1, time.time() - t0)
		ps.append(pred)
	
	with open(config.out(), 'w') as out:
		for y1, p in zip(y, ps):
			write_csv([y1] + p, out)

def print_corrs(corrs):
	for corr in corrs:
		print(corr)
	
	print('-------------')
	print(average(corrs))

def write_results(results, config):
	def convert(s):
		return '-' if s is None else s
	
	samples, y = get_pheno(config)
	corrs = [ calc_corr(y, r) for r in results ]
	print_corrs(corrs)
	
	out = open(config.out(), 'w')
	writer = csv.writer(out)
	num_repeats = len(results)
	trait = config.trait()
	writer.writerow(['sample', trait] + [ '%s_%d' % (trait, i)
									for i in range(1, num_repeats+1) ])
	for k in range(len(y)):
		v = [samples[k], convert(y[k])] + [ convert(p[k]) for p in results ]
		writer.writerow(v)


#################### main ####################

if len(sys.argv) != 2:
	print("usage : predict_SAEnsemble.py config.", file=sys.stderr)
	exit(1)

t0 = time.time()
config = Config(sys.argv[1])
samples, y = get_pheno(config)
CV = CrossValidation(config.path_CV(), y)
optimize(CV, config)
