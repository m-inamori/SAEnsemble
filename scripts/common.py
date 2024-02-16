# coding: utf-8
# common.py

from __future__ import annotations
from typing import Iterator, Any
import csv


#################### library ####################

def read_csv(path: str) -> Iterator[list[str]]:
	with open(path, 'r') as f:
		reader = csv.reader(f)
		for row in reader:
			yield row

def write_csv(table: list[list[Any]], path_out: str):
	with open(path_out, 'w') as out:
		for v in table:
			print(','.join(map(str, v)), file=out)

