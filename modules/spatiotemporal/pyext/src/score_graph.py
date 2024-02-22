import numpy as np

def get_graph_as_dict(nodes):
    """
    converts a list of graphNode objects (nodes), which have been initiated with scores and edges into a dictionary representation of a graph (graph). Each node in the graph is a key, which returns edges in the next state.
    @param nodes: list of graphNode objects
    @return graph: dictionary where each node is a key and the values are the edges in the graph for that node
    """
    graph = {}

    for node in nodes:
        graph[node] = node.get_edges()

    return graph

def find_all_paths(graph, start, end, path=[]):
    """
    Finds all paths between nodes, which already have edges drawn between them.
    @param graph: dictionary representation of the graph, acquired in get_graph_as_dict()
    @param start: graphNode, candidate for starting the graph
    @param end: graphNode, candidate to end the graph
    @param path: list of graphNodes on the path, which is defined recursively.
    @return paths: list of all paths that exist between the starting node and ending node
    """

    path = path + [start]
    if start == end:
        return [path]
    if start not in graph.keys():
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
    """
    Function to score a graph based on nodes, which has scores and edges, as well as keys, which is a list of the states visited. Note that all edges must be drawn and scores must be added to nodes before calling this function.
    @param nodes: list of graphNode objects, which has been initialized with all weights and edges
    @param keys: list of all ordered states (strings) visited along the graph. Paths will be determined in sequential order passed to this function.
    @return all_paths: list of all paths through the graph. Each path is a list of graphNode objects that correspond to the states visited along the path.
    @return path_prob: list of probabilities for each path, ordered in the same order as all_paths
    @return path_scores: list of tuples, where the first object is the path (list of graphNode objects for each state along the trajectory), and the second object is the score of the path, which can be used to calculate the probability.
    """
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