import matplotlib
matplotlib.use("agg")

import skopt
import subprocess

def get_func(mode):
	def eval_thing(params):
		params = list(map(int, params))
		print("trying ", params)
		proc = subprocess.Popen([
			"./main", mode, *map(str, params)], stdout=subprocess.PIPE)
		while True:
			line = proc.stdout.readline().decode()
			print("got line:", line)
			if line.strip().endswith("milliseconds"):
				return int(line.split(" ")[0])
			if line == "":
				raise ValueError()
	return eval_thing

skopt.gp_minimize(
	get_func("1"),
	[
		(0, 8),
		(1, 1e3)
		],
	verbose=10000)

