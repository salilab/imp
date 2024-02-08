import numpy as np
try:
    from graphviz import Digraph
except ImportError:
    print('graphviz not available, will not be able to draw graph')
    print('Errors may occur in create_DAG if draw_dag==True')
try:
    from matplotlib import cm
    from matplotlib import colors as clr
except ImportError:
    print('graphviz not available, will not be able to draw graph')
    print('Errors may occur in create_DAG if draw_dag==True')

# Text / probability output --------------------------------------------------------------------------------------------

# write_cdf - function to output the cumulative distribution function (cdf) if out_cdf is True
def write_cdf(out_cdf,cdf_fn,graph_prob):
    if out_cdf:
        cdf = np.cumsum(np.flip(np.sort(graph_prob)))
        np.savetxt(cdf_fn, cdf)
    return

# write_pdf - function to output the probability distribution function (pdf) if out_pdf is True
def write_pdf(out_pdf,pdf_fn,graph_prob):
    if out_pdf:
        pdf = np.flip(np.sort(graph_prob))
        np.savetxt(pdf_fn,pdf)
    return

# write_labeled_pdf - function to output the labeled probability distribution function (pdf) if out_labeled_pdf is True
def write_labeled_pdf(out_labeled_pdf,labeled_pdf_fn,graph,graph_prob):
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
    return

# function to output a file with all states for each of the n most likely paths
def write_final_npaths(npaths,npath_fn,graph_scores,graph_prob):
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
    return

# Rendering DAG --------------------------------------------------------------------------------------------------------
def draw_dag_in_graphviz(nodes, coloring=None, draw_label=True, fontname="Helvetica", fontsize="18", penscale=0.6, arrowsize=1.2, height="0.6",width="0.6"):
    """Draw a DAG representation in graphviz and return the resulting Digraph.
    Takes a list of graphNodes and initializes the nodes and edges.
    Coloring is expected to be a list of RGBA strings specifying how to color
    each node. Expected to be same length as nodes.
    """

    # create a dot object for the graph
    dot = Digraph(format="eps",
                  engine="dot")
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
    """Render the DAG with heatmap information.

    dag_fn is a string with the filename path.

    nodes is a list of graphNode objects.

    paths is a list of lists containing the paths.
    """

    # determines if heatmap will be overlayed on top of DAG
    if heatmap:

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

    return