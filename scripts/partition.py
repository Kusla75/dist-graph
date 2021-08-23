import random
import os

def split_list(ls, n):
    '''Splits list in n part'''

    x = len(ls)//n # number of nodes in every partition

    p = [ls[i:i + x] for i in range(0, len(ls), x)]

    # if last partion has less elements than other partitions,
    # it's elems are moved to other partitions
    if len(ls) % n != 0:
        it = 0
        for elem in p[-1]:
            p[it].append(elem)
            it += 1
        del p[-1]

    return p

def create_dict_from_nodes_ls(G, nodes_list):
    '''Takes every node in partition and creates dictionary.
    Key is node label and value is list of it's neighbors'''

    partitions = []
    node_dict = {}

    for i in range(len(nodes_list)):
            for node in nodes_list[i]:
                node_dict[node] = list(G.neighbors(node))
            partitions.append(node_dict.copy())
            node_dict.clear()

    return partitions

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
    for every node. Using that list of graph partitions is created.
    
    k = number of copies of each node'''

    nodes_list = list(G.nodes)

    random.seed(16)
    random.shuffle(nodes_list)
    
    if k == 1:
        nodes_list = split_list(nodes_list, n_partitions)
    else:
        nodes_list = clone_nodes(nodes_list, n_partitions, k)

    partitions = create_dict_from_nodes_ls(G, nodes_list)
        
    return partitions

def node_deg_partitioning(G, n_partitions, k = 1):
    pass

def clone_nodes(nodes_list, n_partitions, k):
    '''Clones nodes to each partition. Each node is cloned k times 
    and there isn't any partition with duplicates'''

    ls = []
    for i in range(n_partitions):
        ls.append([])

    for node in nodes_list:
        it = k
        while it > 0:
            id_min = find_min_num_partiton(ls)
            if node not in ls[id_min]:
                ls[id_min].append(node)
            it -= 1

    return ls
            
def find_min_num_partiton(partitions):
    '''Finds partition with minimun number of nodes'''

    minimum = len(partitions[0])
    id_min = 0

    for id in range(len(partitions)):
        if minimum >= len(partitions[id]):
            minimum = len(partitions[id])
            id_min = id

    return id_min

def write_partitions(partitions, ds_name, n_partitions, k):
    '''Saves every partition to different file, labeled with different id'''

    path = os.path.dirname(__file__)
    path = os.path.split(path)[0]
    path += "\\partitions\\" + ds_name
    
    if not os.path.exists(path):
        os.mkdir(path)

    f_line = ""

    dir_name = path + "\\N" + str(n_partitions) + "_"
    dir_name += "K" + str(k) + '\\'
    os.mkdir(dir_name)

    for id in range(len(partitions)):
        with open(dir_name + str(id) + '.txt', 'w+') as f:
            for node, adj_list in partitions[id].items():
                f_line = node + ','
                for n in adj_list:
                    f_line += ' ' + n
                f_line += '\n'
                f.write(f_line)
