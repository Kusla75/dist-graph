import random
import os
from datetime import datetime

def split_list(ls, num):
    '''Splits list in num part'''

    x = len(ls)//num # number of nodes in every partition

    p = [ls[i:i + x] for i in range(0, len(ls), x)]

    if len(ls) % num != 0:
        it = 0
        for elem in p[-1]:
            p[it].append(elem)
            it += 1
        del p[-1]

    return p

def read_dataset(G, dataset_path):
    '''Reads dataset from file and save to networkx graph'''

    with open(dataset_path, 'r') as f:
        lines = f.readlines()
        if ' ' in lines[0]: sep = ' '
        elif '\t' in lines[0]: sep = '\t'
        elif ',' in lines[0]: sep = ','
        else: sep = ''

        for line in lines:
            line = line.strip('\n')
            edge = line.split(sep)
            G.add_edge(edge[0], edge[1])

    return G

def random_shuffle(G, n_partitions, k = 1):
    '''Gets list of nodes in a graph and creates a list of neighbors
    for every node. Using that list of graph partitions is created.'''

    partitions = []
    nodes_dict = {}
    nodes_list = list(G.nodes)

    random.seed(16)
    random.shuffle(nodes_list)
    
    if k == 1:
        nodes_list = split_list(nodes_list, n_partitions)
        for i in range(len(nodes_list)):
            for node in nodes_list[i]:
                nodes_dict[node] = list(G.neighbors(node))
            partitions.append(nodes_dict.copy())
            nodes_dict.clear()
    else:
        for i in range(n_partitions):
            partitions.append([])
        
    
    return partitions

def write_partitions(partitions, n_partitions, k_param):
    '''Saves every partition to different file, labeled with different id'''

    path = ".\\partitions\\"
    f_line = ""

    now_time = datetime.now()
    now_time = now_time.strftime("%d-%m-%Y_%H.%M.%S")

    dir_name = path + now_time 
    dir_name += "_N=" + str(n_partitions) + "_"
    dir_name += "_K=" + str(k_param) + '\\'
    os.mkdir(dir_name)

    for id in range(n_partitions):
        with open(dir_name + str(id) + '.txt', 'w+') as f:
            for node, adj_list in partitions[id].items():
                f_line = node + ','
                for n in adj_list:
                    f_line += ' ' + n
                f_line += '\n'
                f.write(f_line)
                
    