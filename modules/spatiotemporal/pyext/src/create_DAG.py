"""@namespace IMP.spatiotemporal.create_DAG
   Simplified function for creating a spatiotemporal model.
"""
import os
import itertools
import warnings
from IMP.spatiotemporal import graphNode
from IMP.spatiotemporal.score_graph import score_graph
from IMP.spatiotemporal import write_output
from IMP.spatiotemporal import composition_scoring


def create_DAG(state_dict,
               # optional inputs related to model input / calculation
               input_dir='', scorestr='_scores.log', output_dir='',
               # optional inputs related to spatiotemporal scoring
               # (only allowing associative transitions).
               spatio_temporal_rule=False, subcomplexstr='.config',
               expected_subcomplexes=[],
               # optional inputs related to composition scores
               score_comp=False, exp_comp_map={},
               # optional inputs related to model output
               out_cdf=True, out_labeled_pdf=True, out_pdf=False, npaths=0,
               # optional inputs related to DAG output
               draw_dag=True):
    """
    This functions streamlines the process of creating a graph by performing
    all the necessary steps and saving relevant input to files. Features of
    this function are walked through in
    example/toy/Simple_spatiotemporal_example.py

    @param state_dict: dictionary that defines the spatiotemporal model.
           The keys are strings that correspond to each time point in the
           stepwise temporal process. Keys should be ordered according to the
           steps in the spatiotemporal process. The values are integers that
           correspond to the number of possible states at that timepoint.
           Scores for each model are expected to be stored as
           $state_$timescorestr, where state are integers 1->value of the
           dictionary, time is the key in the dictionary, and scorestr is
           trailing characters, which are assumed to be constant for
           all states.
    @param input_dir: string, directory where the data is stored. Empty string
           assumes current working directory.
    @param scorestr: string, trailing characters at the end of the file with
           scores for each stage of the spatiotemporal model
           (default: '_scores.log').
    @param output_dir: string, directory where the output will be written.
           Empty string assumes the same directory as the input_dir.
    @param spatio_temporal_rule: Boolean. If true, enforces that all components
           earlier in the assembly process are present later in the process.
           (default: False)
    @param subcomplexstr: string, trailing characters after the subcomplex
           file, which is a list of subcomplexes included in the given
           label/time (default: '.config')
    @param expected_subcomplexes: list of all possible subcomplex strings
           in the model (default: []) Should be a list without duplicates of
           all components in the subcomplex files.
    @param score_comp: Boolean to determine whether or not to score models
           based on the protein composition.
    @param exp_comp_map: dictionary for determining protein composition score.
           The keys are the proteins. The code checks if the name of these
           proteins are within the subcomplex_components for each node.
           As such, the naming scheme should be such that the keys of
           exp_comp_map are substrings of expected_subcomplexes the values of
           exp_comp_map should correspond to a csv file for each subcomplex
           with protein copy numbers. Each csv file should have 3 columns:
           1) 'Time' - should correspond to the keys of state_dict,
           2) 'mean' - mean copy number from experimental data, and
           3) std - standard deviation from experimental data
    @param out_cdf: Boolean to determine whether or not to write out the
           cumulative distribution function (cdf) for the graph
           (default: True). filename: "cdf.txt"
    @param out_labeled_pdf: Boolean to determine whether to output the
           labeled pdf file, which includes both the pdf and the ordered
           states visited along each path (default: True).
           filename: "labeled_pdf.txt"
    @param out_pdf: Boolean to determine whether or not to write out the
           probability distribution function (pdf) for the graph
           (default: False) filename: "pdf.txt"
    @param npaths: int, write out the states along the n most likely paths,
           based on the pdf (default: 0) filename: "pathXX.txt", where XX
           is the number of the path
    @param draw_dag: Boolean to determine whether or not to write out a
           directed acyclic graph (dag) to a file (default: True)
           filename: "dag_heatmap"
    @return nodes: list of graphNode objects, corresponding to the snapshot
            models in the spatiotemporal model
    @return graph: list of all paths through the graph. Each path is a list
            of graphNode objects that correspond to the states visited
            along the path.
    @return graph_prob: list of probabilities for each path, ordered in the
            same order as all_paths
    @return graph_scores: list of tuples, where the first object is the
            path (list of graphNode objects for each state along the
            trajectory), and the second object is the score of the path,
            which can be used to calculate the probability.
    """

    # Set manual parameters
    # cdf_fn - string, name of the file for the cdf
    cdf_fn = 'cdf.txt'
    # labeled_pdf_fn - string, name of the file for the labeled pdf
    labeled_pdf_fn = 'labeled_pdf.txt'
    # pdf_fn - string, name of the file for the pdf
    pdf_fn = 'pdf.txt'
    # npath_fn - string, name of the file for each pathway
    npath_fn = 'path'

    # dag_fn - string, filename for the dag image (default: 'dag_heatmap')
    dag_fn = 'dag_heatmap'
    # dag_heatmap - Boolean to determine whether or not to write the dag
    # with a heatmap based on the probability of each state (default: True)
    dag_heatmap = True
    # dag_colormap - string, colormap used by the dag to represent probability.
    # Chooses from those available in matplotlib
    # (https://matplotlib.org/stable/users/explain/colors/colormaps.html)
    # (default: "Purples").
    dag_colormap = "Purples"
    # dag_draw_label - Boolean to determine whether or not to draw state
    # labels on the dag
    dag_draw_label = True
    # dag_fontname - string, font used for the labels on the dag
    dag_fontname = "Helvetica"
    # dag_fontsize - string, font size used for the labels on the dag
    dag_fontsize = "18"
    # dag_penscale - float, size of the pen used to draw arrows on the dag
    dag_penscale = 0.6
    # dag_arrowsize - float, size of arrows connecting states on the dag
    dag_arrowsize = 1.2
    # dag_height - string, height of each node on the dag
    dag_height = "0.6"
    # dag_width - string, width of each node on the dag
    dag_width = "0.6"

    # Assert that all inputs are the correct variable type
    if not isinstance(state_dict, dict):
        raise TypeError("state_dict should be of type dict")
    if not isinstance(input_dir, str):
        raise TypeError("input_dir should be of type str")
    if not isinstance(scorestr, str):
        raise TypeError("scorestr should be of type str")
    if not isinstance(spatio_temporal_rule, bool):
        raise TypeError("state_dict should be of type bool")
    if not isinstance(subcomplexstr, str):
        raise TypeError("subcomplexstr should be of type str")
    if not isinstance(expected_subcomplexes, list):
        raise TypeError("expected_subcomplexes should be of type list")
    if not isinstance(score_comp, bool):
        raise TypeError("score_comp should be of type bool")
    if not isinstance(exp_comp_map, dict):
        raise TypeError("exp_comp_map should be of type dict")
    if not isinstance(out_cdf, bool):
        raise TypeError("out_cdf should be of type bool")
    if not isinstance(out_labeled_pdf, bool):
        raise TypeError("out_labeled_pdf should be of type bool")
    if not isinstance(out_pdf, bool):
        raise TypeError("out_pdf should be of type bool")
    if not isinstance(npaths, int):
        raise TypeError("npaths should be of type int")
    if not isinstance(draw_dag, bool):
        raise TypeError("draw_dag should be of type bool")

    # check proteins in the exp_comp_map exist in expected_complexes
    for key in exp_comp_map.keys():
        found = 0
        for subcomplex in expected_subcomplexes:
            if key in subcomplex:
                found = found + 1
        if found == 0:
            warnings.warn(
                'WARNING!!! Check exp_comp_map and expected_subcomplexes. '
                'protein ' + key + ' is not found in expected_subcomplexes. '
                'This could cause illogical results.')

    # Step 1: Initialize graph with static scores
    # list of all nodes
    print('Initialing graph...')
    nodes = []
    # keys correspond to all timepoints
    keys = list(state_dict.keys())
    # Go to input_dir, if it exists
    if len(input_dir) > 0:
        if os.path.exists(input_dir):
            os.chdir(input_dir)
        else:
            raise Exception(
                "Error!!! Does not exist: " + input_dir + '\nClosing...')

    # Loop over all keys and all states
    for key in keys:
        for i in range(state_dict[key]):
            index = i + 1
            node = graphNode.graphNode()
            node.init_graphNode(key, str(index), scorestr, subcomplexstr,
                                expected_subcomplexes)
            nodes.append(node)

    # build up candidate edges in graph
    tpairs = [(keys[i], keys[i + 1]) for i in range(0, len(keys) - 1)]
    for a, b in tpairs:
        # get time marginalized nodes
        anode = [n for n in nodes if n.get_time() == a]
        bnode = [n for n in nodes if n.get_time() == b]
        # draw edges between pairs. Include whether or not to include
        # spatio_temporal_rule
        for na, nb in itertools.product(anode, bnode):
            graphNode.draw_edge(na, nb, spatio_temporal_rule)
    # set indeces for all nodes. These are unique for each node,
    # unlike labels, which can overlap
    for ni, node in enumerate(nodes):
        node.set_index(ni)
    print('Done.')

    # Step 2: Add composition static score to graph
    if score_comp:
        print('Calculation composition likelihood...')
        nodes = composition_scoring.calc_likelihood(exp_comp_map, nodes)
        print('Done.')

    # Step 3: Compute all paths, as well as their scores
    print('Scoring directed acycling graph...')
    graph, graph_prob, graph_scores = score_graph(nodes, keys)
    print('Done.')

    # Step 4: Draw DAG and save relevant output
    print('Writing output...')
    # Go to output directory
    if len(output_dir) > 0:
        if os.path.exists(output_dir):
            os.chdir(output_dir)
        else:
            os.mkdir(output_dir)
            os.chdir(output_dir)
    write_output.write_cdf(out_cdf, cdf_fn, graph_prob)
    write_output.write_pdf(out_pdf, pdf_fn, graph_prob)
    write_output.write_labeled_pdf(out_labeled_pdf, labeled_pdf_fn, graph,
                                   graph_prob)
    write_output.write_final_npaths(npaths, npath_fn, graph_scores, graph_prob)
    if draw_dag:
        write_output.draw_dag(
            dag_fn, nodes, graph, graph_prob, keys, heatmap=dag_heatmap,
            colormap=dag_colormap, draw_label=dag_draw_label,
            fontname=dag_fontname, fontsize=dag_fontsize,
            penscale=dag_penscale, arrowsize=dag_arrowsize, height=dag_height,
            width=dag_width)
    print('Done.')

    return nodes, graph, graph_prob, graph_scores
