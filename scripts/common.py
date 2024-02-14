# coding: utf-8
# common.py
# SAの結果の上位でアンサンブル予測する
# C++によるLibraryを使う

from __future__ import annotations
from typing import Iterator
import csv


#################### library ####################

def read_csv(path: str) -> Iterator[list[str]]:
	with open(path, 'r') as f:
		reader = csv.reader(f)
		for row in reader:
			yield row


