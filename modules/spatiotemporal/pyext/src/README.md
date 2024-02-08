# Spatialtemporal scoring in IMP
Here, we describe the spatiotemporal modeling package. The goal of this package is to read in a variety of input data and convert this data into a weighted graph of states. In the following, we go through each script, along with the functions available.

## create_DAG.py

### create_DAG(state_dict)
This functions streamlines the process of creating a graph by performing all the necessary steps and saving relevant input to files. Features of this function are walked through in example/toy/Simple_spatiotemporal_example.ipynb

#### Possible inputs:

#### Inputs related to model input / calculation
state_dict - dictionary that defines the spatiotemporal model. They keys are strings that correspond to each time point in the stepwise temporal process.
Keys should be ordered according to the steps in the spatiotemporal process. The values are integers that correspond to the number of possible states at that timepoint.
Scores for each model are expected to be stored as state_timescorestr,
where state are integers 1->value of the dictionary, time is the key in the dictionary, and
scorestr is trailing characters, which are assumed to be constant for all states.

input_dir - string, directory where the data is stored. Empty string assumes current working directory.

scorestr - string, trailing characters at the end of the file with scores for each stage of the spatiotemporal model (default: '_scores.log').

output_dir - string, directory where the output will be written. Empty string assumes the same directory as the input_dir.

#### Inputs related to spatiotemporal scoring (all optional)
spatio_temporal_rule- Boolean. If true, enforces that all components earlier in the assembly process are present later in the process. (default: False)

subcomplexstr- string, trailing characters after the subcomplex file, which is a list of subcomplexes included in the given label/time (default: '.config')

expected_subcomplexes- list of all possible subcomplex strings in the model (default: []) Should be a list without duplicates of all components in the subcomplex files.

#### Inputs related to composition scores (all optional)
score_comp - Boolean to determine whether or not to score models based on the protein composition

exp_comp_map - dictionary for determining protein composition score. The keys are the proteins. The code checks if the name of these proteins are within the subcomplex_components for each node. As such, the naming scheme should be such that the keys of exp_comp_map are substrings of expected_subcomplexes the values of exp_comp_map should correspond to a csv file for each subcomplex with protein copy numbers. Each csv file should have 3 columns:
1) 'Time' - should correspond to the keys of state_dict, 2) 'mean' - mean copy number from experimental data, and 3) std - standard deviation from experimental data

#### Inputs related to model output (all optional)
out_cdf - Boolean to determine whether or not to write out the cumulative distribution function (cdf) for the graph (default: True)

cdf_fn - string, filename for the cdf (default: 'cdf.txt')

out_labeled_pdf - Boolean to determine whether to output the labeled pdf file, which includes both the pdf and the ordered states visited along each path (default: True).

labeled_pdf_fn - string, name of the file for the labeled pdf (default: 'labeled_pdf.txt')

out_pdf - Boolean to determine whether or not to write out the probability distribution function (pdf) for the graph (default: False)

pdf_fn - string, filename for the pdf (default: 'pdf.txt')

npaths - int, write out the states along the n most likely paths, based on the pdf (default: 0)

npath_fn - string, name of the file for each of the n most likely paths. 'n.txt' will be appended to the end of npath_fn (default: 'path')

#### Inputs related to directed acyclic graph (DAG) output (all optional)
draw_dag - Boolean to determine whether or not to write out a directed acyclic graph (dag) to a file (default: True)

dag_fn - string, filename for the dag image (default: 'dag_heatmap')

dag_heatmap - Boolean to determine whether or not to write the dag with a heatmap based on the probability of each state (default: True)

dag_colormap - string, colormap used by the dag to represent probability. Chooses from those available in matplotlib
(https://matplotlib.org/stable/users/explain/colors/colormaps.html) (default: "Purples").

dag_draw_label - Boolean to determine whether or not to draw state labels on the dag

dag_fontname - string, font used for the labels on the dag

dag_fontsize - string, font size used for the labels on the dag

dag_penscale - float, size of the pen used to draw arrows on the dag

dag_arrowsize - float, size of arrows connecting states on the dag

dag_height - string, height of each node on the dag

dag_width - string, width of each node on the dag

## analysis.py

### temporal_precision(labaled_pdf1_fn,labaled_pdf2_fn,output_fn)
Function that reads in two labeled_pdfs from create_DAG and returns the temporal_precision, defined as the probability overlap between two pathway models.

labaled_pdf1_fn - labeled_pdf from one independent sampling

labaled_pdf2_fn - labeled_pdf from another independent sampling

output_fn - name of output file (default: 'temporal_precision.txt')

### purity(labeled_pdf_fn,output_fn)
Function that reads in one labeled_pdf from create_DAG and returns the purity, defined as the sum of the squared probability of all trajectories.

labeled_pdf_fn - labeled_pdf from the total model

output_fn - name of output file (default: 'temporal_precision.txt')

## graphNode.py

### graphNode
graphNode is a class that defines how states are connected in a spatiotemporal model.

#### a graphNode can possess the following properties:
edges - set, which describes connections to other nodes

scores - list of floats, mean scores based on priors and likelihoods used for modeling

time - string, describes the time of the node

index - int, unique numbering for each node

label - int, numbering for each node that starts at 1 at each time point. This will correspond to the number in the input data

components - List of proteins/subcomplexes in this particular node. Must be a subset of expected_subcomplexes

expected_subcomplexes - list of possible proteins/subcomplexes in the overall system

#### init_graphNode(time, label, scorestr, subcomplexstr, expected_subcomplexes):
Function that initiates a graph node with specific time, label, and expected_subcomplexes. Scores and components are extracted from files named scorestr and subcomplexstr respectively. Returns a single graphNode object.

### draw_edge(nodeA, nodeB, spatio_temporal_rule)
Draws an edge between graphNode objects nodeA and nodeB. If spatio_temporal_rule, node will only be drawn if the components of nodeA are a subset of the components of nodeB.

## composition_scoring.py

### calc_likelihood(exp_comp_map,nodes):
Function that adds a score for the compositional likelihood for all states represented as nodes in the graph. The composition likelihood assumes a Gaussian distribution for copy number of each protein or subcomplex with means and standard deviatiations derived from experiment. Returns the nodes, with the new weights added.

nodes - list of graphNode objects, which have been already been initiated with static scores

exp_comp_map - dictionary, which describes protein stoicheometery. The key describes the protein, which should correspond to names within the expected_subcomplexes. Only copy numbers for proteins or subcomplexes included in this dictionary will be scored. For each of these proteins, a csv file should be provided with protein copy number data. The csv file should have 3 columns, 1) "Time", which matches up to the possible times in the graph, 2) "mean", the average protein copy number at that time point from experiment, and 3) "std", the standard deviation of that protein copy number from experiment.

### composition_likelihood_function(mean, std, prots, node):
Function that calculates the likelihood of an individual node, used by calc_likelihood().

mean - dictionary of dictionaries where the first key is the protein, the second key is the time, and the expected mean copy number from experiment is returned.

std - dictionary of dictionaries where the first key is the protein, the second key is the time, and the expected standard deviation of protein copy number from experiment is returned.

prots - list of proteins or subcomplexes which will be scored according to this likelihood function

node - the graphNode object for which the likelihood will be calculated.

get_state(subcomplex_components,prot):
function to calculate how many times a protein appears in a list of proteins

prot - string, protein or subcomplex we are interested in finding

subcomplex_components - subcomplexes or components in a given node, which can be accessed by graphNode.get_subcomplex_components()

## score_graph.py

### score_graph(nodes,keys):
Function to score a graph based on nodes, which has scores and edges, as well as keys, which is a list of the states visited. Note that all edges must be drawn and scores must be added to nodes before calling this function.

nodes - list of graphNode objects, which has been initialized with all weights and edges

keys - list of all ordered states visited along the graph. Paths will be determined in sequential order passed to this function.

Returns 3 objects: 

all_paths - list of all paths through the graph. Each path is a list of graphNode objects that correspond to the states visited along the path.

path_prob - list of partialities for each path, ordered in the same order as all_paths

path_scores - list of tuples, where the first object is the path (list of graphNode objects for each state along the trajectory), and the second object is the score of the path, which can be used to calculate the probability.

### get_graph_as_dict(nodes):
converts a list of graphNode objects (nodes), which have been initiated with scores and edges into a dictionary representation of a graph (graph). Each node in the graph is a key, which returns edges in the next state.

### find_all_paths(graph, start, end, path=[]):
Finds all paths between nodes, which already have edges drawn between them.

graph - dictionary representation of the graph, acquired in get_graph_as_dict()

start - candidate starting graphNode

end - candidate ending graphNode

path - list of graphNodes on the path, which is defined recursively.

Returns all paths that exist between the starting node and ending node

## write_output.py

### write_cdf(out_cdf,cdf_fn,graph_prob):
function to output the cumulative distribution function (cdf)

out_cdf - bool, writes cdf if true

cdf_fn - str, filename of cdf

graph_prob - list of probabilities for each path, (path_prob from score_graph())

### write_pdf(out_pdf,pdf_fn,graph_prob):
function to output the probability distribution function (pdf)

out_pdf - bool, writes pdf if true

pdf_fn - str, filename of pdf

graph_prob - list of probabilities for each path, (path_prob from score_graph())

### write_labeled_pdf(out_labeled_pdf,labeled_pdf_fn,graph,graph_prob):
function to output the labeled probability distribution function (pdf)

out_labeled_pdf - bool, writes labeled_pdf if true

labeled_pdf_fn - str, filename of labeled_pdf

graph - list of graphNode objects visited for each path, (all_paths from score_graph())

graph_prob - list of probabilities for each path, (path_prob from score_graph())

### write_final_npaths(npaths,npath_fn,graph_scores,graph_prob):
function to output a file with all states for each of the n most likely paths

npaths - int, number of paths to output

nphath_fn - str, name of the file for all paths

graph_scores - list of tuples, where the first object is the path (list of graphNode objects for each state along the trajectory), and the second object is the score of the path, which can be used to calculate the probability. (path_scores from score_graph())

graph_prob - list of probabilities for each path, (path_prob from score_graph())

### draw_dag(dag_fn, nodes, paths, path_prob, key,heatmap=True, colormap="Purples", penscale=0.6, arrowsize=1.2, fontname="Helvetica", fontsize="18", height="0.6", width="0.6", draw_label=True)
Function to render the DAG with heatmap information.

dag_fn is a string with the filename path.

nodes is a list of graphNode objects.

paths is a list of lists containing the paths.

path_prob - list of probabilities for each path, (path_prob from score_graph())

keys - states visited in the graph (list of keys to the state_dict)

heatmap - Boolean to determine whether or not to write the dag with a heatmap based on the probability of each state (default: True)

colormap - string, colormap used by the dag to represent probability. Chooses from those available in matplotlib
(https://matplotlib.org/stable/users/explain/colors/colormaps.html) (default: "Purples").

draw_label - Boolean to determine whether or not to draw state labels on the dag

fontname - string, font used for the labels on the dag

fontsize - string, font size used for the labels on the dag

penscale - float, size of the pen used to draw arrows on the dag

arrowsize - float, size of arrows connecting states on the dag

height - string, height of each node on the dag

width - string, width of each node on the dag

### draw_dag_in_graphviz(nodes, coloring=None, draw_label=True, fontname="Helvetica", fontsize="18", penscale=0.6, arrowsize=1.2, height="0.6",width="0.6"):
Function used by draw_dag() to render the graph using graphviz. Takes a list of graphNodes (nodes) and initializes the nodes and edges. Coloring is expected to be a list of RGBA strings specifying how to color each node. Expected to be same length as nodes.

