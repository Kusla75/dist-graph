#!/bin/env python3

import os

home_dir = os.environ["HOME"]
solution_path = home_dir + "/solutions/"
results_path = home_dir + "/results/"

solution_dic = {}
results_dic = {}
error_nodes = []
num_errors = 0

class colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def load_data(f):
    dic = {}
    with open(f, "r") as f:
        for line in f.readlines():
            line = line.strip("\n")
            line = line.replace(" ", "")
            line = line.split(":")

            flt = float(line[1])
            dic[line[0]] = round(flt, 10)

    return dic

def verify(res_dic, sol_dic):
    error_nodes = []

    for key, val in res_dic.items():
        if abs(val - sol_dic[key]) > 1e-7:
            error_nodes.append(key)

    return error_nodes

for ds_dir in os.listdir(results_path):
    path = os.path.join(solution_path, ds_dir)
    solution_dic = load_data(path + ".txt")

    path = os.path.join(results_path, ds_dir)
    for part_dir in os.listdir(path):
        path = os.path.join(path, part_dir)
        for f_name in os.listdir(path):
            if "res" in f_name:
                results_dic = load_data(os.path.join(path, f_name))
                error_nodes = verify(results_dic, solution_dic)
                if error_nodes != []:
                    print(colors.FAIL + "ERROR:" + colors.ENDC, end=" ")
                    print(colors.HEADER + os.path.join(ds_dir, part_dir, f_name) + colors.ENDC, end=" ")
                    print(error_nodes)
                    num_errors += len(error_nodes)
        path = os.path.split(path)[0]

if num_errors == 0:
    print(colors.OKGREEN + "\nVerification complete. No errors detected" + colors.ENDC)
else:
    print(colors.FAIL + f"\nTotal: {num_errors} errors detected" + colors.ENDC)

print()
