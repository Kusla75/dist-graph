# This script is used to generate for every computational problem.
# Clustering coefficient of each node is written in solution directory.

import networkx as nx
import paths as p
from partition import read_dataset

G = nx.Graph()

ds_file = 'socfb-mich.txt' # Name of file to save solution

G = read_dataset(G, p.socfb_mich)

clustering_coeff_dic = nx.clustering(G)

with open(p.solutions_path + "\\" + ds_file, "w") as f:
    line = ""
    for key, value in clustering_coeff_dic.items():
        line = str(key) + ": " + str(value) + "\n"
        f.write(line)
        line = ""



