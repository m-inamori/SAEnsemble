# coding: utf-8
# CV.py

from __future__ import annotations
from itertools import islice
from typing import Any
import random

from common import read_csv, write_csv


#################### CrossValidation ####################

class CrossValidation:
	def __init__(self, samples: list[str], table: list[list[int]]):
		self.samples: list[str] = samples
		self.CV: list[list[int]] = table
	
	def num_repeats(self):
		return len(self.CV)
	
	def num_divisions(self):
		return max(self.CV[0])
	
	def extract_X_train(self, X: list[list[Any]],
								k: int, g: int) -> list[list[Any]]:
		v = self.CV[k]
		return [ [ x for x, g1 in zip(row, v) if g1 != g ] for row in X ]
	
	def extract_y_train(self, y: list[float], k: int, g: int) -> list[float]:
		v = self.CV[k]
		return [ y1 for y1, g1 in zip(y, v) if g1 != g ]
	
	def extract_X_test(self, X: list[list[Any]],
								k: int, g: int) -> list[list[Any]]:
		v = self.CV[k]
		return [ [ x for x, g1 in zip(row, v) if g1 == g ] for row in X ]
	
	def set_values(self, pred: list[float], p_sub: list[float], k: int, g: int):
		v = self.CV[k]
		for i, value in zip((i for i, g1 in enumerate(v) if g1 == g), p_sub):
			pred[i] = value
	
	def write(self, path: str):
		nrep = self.num_repeats()
		table = [[''] + [ 'CV%d' % i for i in range(1, nrep+1) ]]
		for s, *v in zip(self.samples, *self.CV):
			table.append([s] + list(v))
		write_csv(table, path)
	
	@staticmethod
	def create_randomly(samples: list[str],
						ndiv: int, nrep: int) -> CrossValidation:
		N = len(samples)
		CV: list[list[int]] = []
		for i in range(nrep):
			v = [ g+1 for g in range(ndiv)
						for _ in range(g*N//ndiv, (g+1)*N//ndiv) ]
			random.shuffle(v)
			CV.append(v)
		return CrossValidation(samples, CV)
	
	@staticmethod
	def read(path: str) -> CrossValidation:
		table = list(read_csv(path))
		samples = [ v[0] for v in table[1:] ]
		CV = [ [ int(table[j][i]) for j in range(1, len(table)) ]
										for i in range(1, len(table[0])) ]
		return CrossValidation(samples, CV)

