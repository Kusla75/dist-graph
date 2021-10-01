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
    
    nodes_list = list(G.nodes)

    random.seed(1616)
    partitions = [ [] for _ in range(n_partitions) ]

    for node in nodes_list:
        r = random.randint(0, n_partitions-1)
        partitions[r].append(node)

    if k > 1:
        pass

    partitions = bad_sort(partitions)
    partitions = create_partitions_dict(G, partitions)

    return partitions

def node_num_partitioning(G, n_partitions, k = 1):
    '''Gets list of nodes in a graph and creates a list of neighbors
    for every node. Using that list of graph partitions is created.
    
    k = number of copies of each node'''

    nodes_list = list(G.nodes)

    random.seed(1616)
    random.shuffle(nodes_list)
    
    partitions = split_list(nodes_list, n_partitions)
 
    # Cloning nodes
    if k > 1:
        for node in nodes_list:
            for _ in range(k-1):
                id_min = find_min_size_partition(partitions, node)
                partitions[id_min].append(node)

    partitions = bad_sort(partitions) # Slow to execute

    # s = 0
    # for ind in range(len(partitions)):
    #     for n in partitions[ind]:
    #         s += G.degree[n]
    #     print(f"{ind} {len(partitions[ind])} {s}")
    #     s = 0

    partitions = create_partitions_dict(G, partitions)
        
    return partitions

def node_deg_partitioning(G, n_partitions, k = 1):
    '''Creates node partitions based on deg of each node'''

    nodes_list = list(G.nodes)
    partitions = []
    par_deg_sum = []

    for _ in range(n_partitions): 
        partitions.append([])
        par_deg_sum.append(0)

    for node in nodes_list:
        for _ in range(k):
            id_min = find_min_node_deg_partition(partitions, node, par_deg_sum)
            partitions[id_min].append(node)
            par_deg_sum[id_min] += G.degree[node]

    partitions = bad_sort(partitions)

    # s = 0
    # for ind in range(len(partitions)):
    #     for n in partitions[ind]:
    #         s += G.degree[n]
    #     print(f"{ind} {len(partitions[ind])} {s}")
    #     s = 0

    partitions = create_partitions_dict(G, partitions)

    return partitions

def find_min_size_partition(partitions, node):
    '''Finds partition with minimum number of nodes 
    that doesn't have a given node in it. Returns id of that partition.'''

    min_size = 999999999
    id_min = -1

    for i in range(len(partitions)):
        if min_size >= len(partitions[i]) and node not in partitions[i]:
            min_size = len(partitions[i])
            id_min = i

    return id_min

def find_min_node_deg_partition(partitions, node, par_deg_sum):
    '''Finds partition with minimum of node degree
    that doesn't have a given node in it. Returns id of that partition.'''

    min_deg = 999999999
    id_min = -1
    
    for ind, s in enumerate(par_deg_sum):
        if min_deg >= s and node not in partitions[ind]:
            min_deg = s
            id_min = ind

    return id_min

def write_partitions(partitions, ds_name, part_method, n_partitions, k):
    '''Saves every partition to different file, labeled with different id'''

    path = os.path.dirname(__file__)
    path = os.path.split(path)[0]
    path += "\\partitions\\" + ds_name
    
    if not os.path.exists(path):
        os.mkdir(path)

    f_line = ""

    dir_name = path + "\\N" + str(n_partitions) + "_"
    dir_name += "K" + str(k) + "_" + part_method + "\\"
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