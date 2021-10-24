# This script is used to parse datasets and create txt files that represent
# partitions of a graph. Each partition will be assigned to each worker in distributed system

import networkx as nx
import paths as p
import partition as par

G = nx.Graph()
edge = []

ds_name = 'fb-pages-food' # dataset that will be used
n_partitions = 0 # number of partitions
k = 0 # number of copies for every node 

G = par.read_dataset(G, p.fb_pages_food_path)

n_partitions = input("Num of partitions: ")
n_partitions = int(n_partitions)

k = input("K param (default 1): ")
if k != '':
    k = int(k)
else:
    k = 1

part_method = input("Partitioning method: ")
if part_method == 'node_deg':
    partitions = par.node_deg_partitioning(G, n_partitions, k)
elif part_method == 'node_num':
    partitions = par.node_num_partitioning(G, n_partitions, k)
elif part_method == 'rand':
    partitions = par.random_partitioning(G, n_partitions, k)
else:
    partitions = par.node_num_partitioning(G, n_partitions, k)

par.write_partitions(partitions, ds_name, part_method, n_partitions, k)
