def attribute_fill(fp,fn):
	for each in fp:
		end = each.find(" ")
		attribute = each[:end:]
		if attribute in attr_list:
			h = each.find("#")
			val = each[end:h:]
			# print(attribute)
			# print(val)
			try:
				val = val.strip().rstrip()
				val = val.rstrip('0').rstrip('.') if '.' in s else s
				if '.' in val:
					val = float(val)
				else:
					val = int(val)
			except Exception as e:
				# print(e.__class__)
				val = list(filter(lambda x:x!='',val.split(" ")))
				val = " ".join(val)

			for i in attr:
				if attribute==i[0]:
					i[fn] = val

import argparse
import csv

parser = argparse.ArgumentParser()

parser.add_argument("--file-path", type=str)
parser.add_argument("--attr-path", type=str)
parser.add_argument("--output-path", type=str)
args = parser.parse_args()

files = list(args.file_path.split(";"))
attr_path = args.attr_path
attr = []
attr_list = []

if not attr_path:
	print("\nEnter comparison attributes: (Type end to stop)")
	inp = input()
	while inp!="end":
		attr.append([inp,'NA','NA','NA'])
		attr_list.append(inp)
		inp = input()
else:
	a = open(attr_path,"r")
	for each in a:
		# print(list(each))
		if '\n' in each:
			each = each[:len(each)-1:]
			each = each.rstrip()
		attr.append([each,'NA','NA','NA'])
		attr_list.append(each)

f1 = open(files[0],"r")
f2 = open(files[1],"r")
f3 = open(files[2],"r")

attribute_fill(f1,1)
attribute_fill(f2,2)
attribute_fill(f3,3)

op = open(args.output_path,"w")
l = "\t\t\tValues are DDR, HMC, HMC+PIM from left to right\n\n"
op.write(l)
l = "---------------------------------------------------------------------------------------------------------------\n\n"
op.write(l)
for out in attr:
	l = out[0] + '\t\t\t' + out[1] + '\t' + out[2] +'\t' + out[3] + '\n\n'
	op.write(l)

f1.close()
f2.close()
f3.close()
op.close()