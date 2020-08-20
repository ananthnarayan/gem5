import random
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--file-size", type=int)
args = parser.parse_args()

file = open("./merkle/input_gen.txt","w")
inp = "qwertyuiopasdfghjklzxcvbnm0123456789"
st = ""
for _ in range(args.file_size):
	rn = random.randint(0,35)
	st = st + inp[rn]

file.write(st)
file.close()
