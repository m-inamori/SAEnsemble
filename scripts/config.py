# coding: utf-8
# config.py

from __future__ import annotations
import sys


#################### ConfigBase ####################

class ConfigBase:
	def __init__(self, path: str):
		self.dic: dict[str, str] = { }
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
	
	def check_existence(self, *keys) -> bool:
		OK = True
		for key in keys:
			if key not in self.dic:
				OK = False
				print("key %s must exist." % key, file=sys.stderr)
		return OK
	
	def check_existence_int(self, *keys) -> bool:
		OK = True
		for key in keys:
			if key not in self.dic:
				OK = False
				print("key %s must exist." % key, file=sys.stderr)
			elif ConfigCheckable.is_int(key):
				OK = False
				print("key %s must be integer." % key, file=sys.stderr)
		return OK
	
	def check_int(self, *keys) -> bool:
		OK = True
		for key in keys:
			if key in self.dic and not ConfigCheckable.is_int(self.dic[key]):
				OK = False
				print("key %s must be integer." % key, file=sys.stderr)
		return OK
	
	@staticmethod
	def is_int(s: str) -> bool:
		try:
			int(s)
			return True
		except ValueError:
			return False
