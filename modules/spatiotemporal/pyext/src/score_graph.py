import numpy as np

def get_graph_as_dict(nodes):
    """Return a dictionary representation of the graph defined by the list of
    nodes.
    """
    graph = {}

    for node in nodes:
        graph[node] = node.get_edges()

    return graph

def find_all_paths(graph, start, end, path=[]):
    """Return a list of all paths through the graph between two points on the
    graph.
    """

    path = path + [start]
    if start == end:
        return [path]
    if not start in graph.keys():
        return []
    paths = []
    for node in graph[start]:
        if node not in path:
            newpaths = find_all_paths(graph, node, end, path)
            for newpath in newpaths:
                paths.append(newpath)

    return paths

# Function to score a graph based on nodes, which has scores and edges, as well as keys, which is a list of the states visited
def score_graph(nodes,keys):
    # nodes - graphNode object, which has been initialized with all weights and edges
    # keys - Ordered list of all states. Paths will be determined in sequential order passed to this function.

    # Determine starting state and final state
    time_start=keys[0]
    time_end=keys[-1]

    # enumerate all paths by iterating over all possible starting and ending points
    starting_nodes = [n for n in nodes if n.get_time() == time_start]

    # get mature pore
    ending_nodes = [n for n in nodes if n.get_time() == time_end]

    graph = get_graph_as_dict(nodes)

    all_paths = []
    for sn in starting_nodes:
        for en in ending_nodes:
            all_paths += find_all_paths(graph, sn, en)

    # compute all path scores as a np array.
    path_scores = [(path, np.array([n.get_weight() for n in path]).sum()) for path in all_paths]
    s = np.array([p[1] for p in path_scores])
    s -= s.min()
    path_prob = np.exp(-s) / np.exp(-s).sum()

    return all_paths,path_prob,path_scores