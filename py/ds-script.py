# This script is used to parse datasets and create txt files that represent
# partitions of a graph. Each partition will be assigned to each worker in distributed system

import networkx as nx
import paths as p
import partition as par

G = nx.Graph()
edge = []

ds_name = 'soc-karate' # dataset that will be used

n_partitions = 0 # number of partitions
k = 0 # number of copies for every node 

n_partitions = input("Num of partitions: ")
n_partitions = int(n_partitions)
k = input("K param (default 1): ")
if k != '':
    k = int(k)
else:
    k = 1

G = par.read_dataset(G, p.karate_path)

partitions = par.random_partitioning(G, n_partitions, k)

par.write_partitions(partitions, ds_name, n_partitions, k)
