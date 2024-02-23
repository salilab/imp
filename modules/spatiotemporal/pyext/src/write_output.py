import numpy as np
try:
    from graphviz import Digraph
except ImportError:
    Digraph = None
try:
    from matplotlib import cm
    from matplotlib import colors as clr
except ImportError:
    cm=None
    clr=None

# Text / probability output --------------------------------------------------------------------------------------------

def write_cdf(out_cdf,cdf_fn,graph_prob):
    """
    Function to output the cumulative distribution function (cdf)
    @param out_cdf: bool, writes cdf if true
    @param cdf_fn: str, filename of cdf
    @param graph_prob: list of probabilities for each path, (path_prob from score_graph())
    """
    if out_cdf:
        cdf = np.cumsum(np.flip(np.sort(graph_prob)))
        np.savetxt(cdf_fn, cdf)

def write_pdf(out_pdf,pdf_fn,graph_prob):
    """
    Function to output the probability distribution function (pdf)
    @param out_pdf: bool, writes pdf if true
    @param pdf_fn: str, filename of pdf
    @param graph_prob: list of probabilities for each path, (path_prob from score_graph())
    """
    if out_pdf:
        pdf = np.flip(np.sort(graph_prob))
        np.savetxt(pdf_fn,pdf)

def write_labeled_pdf(out_labeled_pdf,labeled_pdf_fn,graph,graph_prob):
    """
    Function to output the labeled probability distribution function (pdf)
    @param out_labeled_pdf: bool, writes labeled_pdf if true
    @param labeled_pdf_fn: str, filename of labeled_pdf
    @param graph: list of graphNode objects visited for each path, (all_paths from score_graph())
    @param graph_prob: list of probabilities for each path, (path_prob from score_graph())
    """
    if out_labeled_pdf:
        # open file
        new = open(labeled_pdf_fn, 'w')
        new.write('#\tPath\t\tpdf\n')
        # loop over all paths in the graph
        for i in range(0,len(graph_prob)):
            # get index for the ith most likely path
            pdf_index = np.flip(np.argsort(graph_prob))[i]
            path=graph[pdf_index]
            # get all labels / time for the ith most likely path
            all_labels=''
            for node in path:
                all_labels += node.get_label() + '_' + node.get_time() + '|'
            # write that path to a new file
            new.write(all_labels + '\t' + str(graph_prob[pdf_index]) + '\n')
        new.close()

def write_final_npaths(npaths,npath_fn,graph_scores,graph_prob):
    """
    Function to output a file with all states for each of the n most likely paths
    @param npaths: int, number of paths to output
    @param npath_fn: str, name of the file for all paths
    @param graph_scores: list of tuples, where the first object is the path (list of graphNode objects for each state along the trajectory), and the second object is the score of the path, which can be used to calculate the probability. (path_scores from score_graph())
    @param graph_prob: list of probabilities for each path, (path_prob from score_graph())
    """
    # loop over npaths
    for i in range(-1, -1*npaths-1, -1):
        path = []
        # get index for sorted probability
        m = np.argsort(graph_prob)[i]
        # go to that index and grab the path
        for node in graph_scores[m][0]:
            # append times not yet in the path
            if node.get_time() not in path:
                path.append(node.get_label()+'_'+node.get_time())

        # save to new file
        with open(npath_fn + str(abs(i)) + ".txt", "w") as fh:
            for statename in path:
                fh.write(statename + "\n")

# Rendering DAG --------------------------------------------------------------------------------------------------------
def draw_dag_in_graphviz(nodes, coloring=None, draw_label=True, fontname="Helvetica", fontsize="18", penscale=0.6, arrowsize=1.2, height="0.6",width="0.6"):
    """Draw a DAG representation in graphviz and return the resulting Digraph.
    Takes a list of graphNodes and initializes the nodes and edges.
    Coloring is expected to be a list of RGBA strings specifying how to color
    each node. Expected to be same length as nodes.
    @param nodes: list of graphNode objects
    @param coloring: list of RGBA strings to specify the color of each node. Expected to be the same length as nodes
    @param draw_label: bool, whether or not to draw graph labels
    @param fontname: string, name of font for graph labels
    @param fontsize: string, size of font for graph labels
    @param penscale: float, size of pen
    @param arrowsize: float, size of arrows
    @param height: string, height of nodes
    @param width: string, width of nodes
    @return dot: Digraph object to be rendered
    """

    if Digraph is None:
        raise Exception("graphviz not available, will not be able to draw graph")
    else:
        # create a dot object for the graph
        dot = Digraph(format="eps",engine="dot")
        dot.attr(ratio="1.5")
        dot.attr(rotate="0")


        for ni, node in enumerate(nodes):
            if coloring is not None:
                color = coloring[ni]
            else:
                color = "#ffffff"

            if draw_label:
                dot.node(str(node),
                     label=node.get_label(),
                     style="filled",
                     fillcolor=color,
                     fontname=fontname,
                     fontsize=fontsize,
                     height=height,
                     width=width)
            else:
                dot.node(str(node),
                     label=' ',
                     style="filled",
                     fillcolor=color,
                     fontname=fontname,
                     fontsize=fontsize,
                     height=height,
                     width=width)

        for ni, node in enumerate(nodes):
            edges = node.get_edges()
            for edge in edges:
                dot.edge(str(node),
                         str(edge),
                         arrowsize=str(arrowsize),
                         color="black",
                         penwidth=str(penscale))

    return dot

# first set of parameters are required and determine the connectivity of the map
def draw_dag(dag_fn, nodes, paths, path_prob, keys,
             # 2nd set of parameters are for rendering the heatmap
             heatmap=True, colormap="Purples", penscale=0.6, arrowsize=1.2, fontname="Helvetica", fontsize="18", height="0.6", width="0.6", draw_label=True):
    """
    Function to render the DAG with heatmap information.
    @param dag_fn: string, filename path
    @param nodes: list of graphNode objects for which the graph will be drawn
    @param paths: list of lists containing all paths visited by the graphNode objects
    @param path_prob: list of probabilities for each path, (path_prob from score_graph())
    @param keys: states visited in the graph (list of keys to the state_dict)
    @param heatmap: Boolean to determine whether or not to write the dag with a heatmap based on the probability of each state (default: True)
    @param colormap: string, colormap used by the dag to represent probability. Chooses from those available in matplotlib (https://matplotlib.org/stable/users/explain/colors/colormaps.html) (default: "Purples").
    @param penscale: float, size of the pen used to draw arrows on the dag
    @param arrowsize: float, size of arrows connecting states on the dag
    @param fontname: string, font used for the labels on the dag
    @param fontsize: string, font size used for the labels on the dag
    @param height: string, height of each node on the dag
    @param width: string, width of each node on the dag
    @param draw_label: Boolean to determine whether or not to draw state labels on the dag
    """

    # determines if heatmap will be overlayed on top of DAG
    if heatmap:

        if cm is None or clr is None:
            raise Exception("matplotlib not available, will not be able to draw graph")
        else:

            default_cmap = cm.get_cmap(colormap)

            # make a list of counts for each node to color
            coloring = np.zeros(len(nodes), dtype=float)
            for path, p in zip(paths, path_prob):
                for n in path:
                    coloring[int(n.get_index())] += 1*p

            # normalize probability
            for t in keys:
                b = np.array([t == n.get_time() for n in nodes])
                coloring[b] /= coloring[b].sum()

            # convert probability to colors
            cmap_colors = [clr.to_hex(default_cmap(color)) for color in coloring]


            dot = draw_dag_in_graphviz(nodes, coloring=cmap_colors, draw_label=draw_label, fontname=fontname, fontsize=fontsize, penscale=penscale, arrowsize=arrowsize,   height=height, width=width)
            dot.render(dag_fn)

    # no heatmap
    else:
        dot = draw_dag_in_graphviz(nodes, coloring=None, draw_label=draw_label, fontname=fontname, fontsize=fontsize, penscale=penscale, arrowsize=arrowsize, height=height, width=width)
        dot.render(dag_fn)