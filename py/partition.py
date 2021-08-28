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

def create_partitions_dict(G, nodes_list):
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
            if edge[0] != edge[1]:
                G.add_edge(edge[0], edge[1])

    return G

def random_partitioning(G, n_partitions, k = 1):
    '''Gets list of nodes in a graph and creates a list of neighbors
    for every node. Using that list of graph partitions is created.
    
    k = number of copies of each node'''

    nodes_list = list(G.nodes)

    random.seed(16)
    random.shuffle(nodes_list)
    
    partitions = split_list(nodes_list, n_partitions)
    
    if k > 1:
        partitions = clone_nodes_by_par_size(G, partitions, n_partitions, k-1)

    partitions = bad_sort(partitions) # Slow to execute

    partitions = create_partitions_dict(G, partitions)
        
    return partitions

def node_deg_partitioning(G, n_partitions, k = 1):
    pass

def clone_nodes_by_par_size(G, partitions, n_partitions, k):
    '''Clones nodes to each partition. Each node is cloned k times 
    and there isn't any partition with duplicate nodes. Each partition will have
    approximately same number of nodes.'''

    for node in list(G.nodes):
        it = 0
        while it < k:
            id_min = find_min_num_partiton_without_node(partitions, node)
            partitions[id_min].append(node)
            it += 1

    return partitions
  
def find_min_num_partiton_without_node(partitions, node):
    '''Finds partition with minimum number of nodes 
    that doesn't have a given node in it. Returns id of that partition.'''

    min_size = 999999999
    id_min = 0

    for i in range(len(partitions)):
        if min_size >= len(partitions[i]) and node not in partitions[i]:
            min_size = len(partitions[i])
            id_min = i

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
    if not os.path.exists(dir_name):
        os.mkdir(dir_name)

    for i in range(len(partitions)):
        with open(dir_name + str(i) + '.txt', 'w+') as f:
            for node, adj_list in partitions[i].items():
                f_line = node + ','
                for n in adj_list:
                    f_line += ' ' + n
                f_line += '\n'
                f.write(f_line)

def bad_sort(partitions):
    '''This sort function is bad because it has to convert elems from str to int,
    sort and then convert elems back from int to str. Bad!!!'''

    ls = []
    for i in range(len(partitions)):
        ls.append([int(n) for n in partitions[i]])
    for elem in ls:
        elem.sort()
    partitions = []
    for i in range(len(ls)):
        partitions.append([str(n) for n in ls[i]])

    return partitions