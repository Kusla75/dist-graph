import networkx as nx
import partition as par

G = nx.Graph()
edge = []

# Dataset paths
karate_path = '.\\datasets\\soc-karate.mtx'
fb_pages_path = '.\\datasets\\fb-pages-government.edges'
pokec_path = '.\\datasets\\soc-pokec-relationships.txt'

n_partitions = 0 # number of partitions
k_param = 0 # number of copies for every node 

n_partitions = input("Num of partitions: ")
n_partitions = int(n_partitions)
k_param = input("K param: ")
if k_param != '':
    k_param = int(k_param)
else:
    k_param = 1

G = par.read_dataset(G, karate_path)

partitions = par.random_shuffle(G, n_partitions, k_param)

par.write_partitions(partitions, n_partitions, k_param)
