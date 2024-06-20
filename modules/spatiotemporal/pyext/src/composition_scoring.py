"""@namespace IMP.spatiotemporal.composition_scoring
   Functions for weighting graphNode objects based on stoichiometry data.
"""
import numpy as np
import warnings
import os


def get_state(subcomplex_components, prot):
    """
    function to calculate how many times a protein appears in a list of
    proteins, which can be accessed from a graphNode object using
    node.get_subcomplex_components()

    @param subcomplex_components: subcomplexes or components in a given node,
           which can be accessed by graphNode.get_subcomplex_components()
    @param prot: string, protein or subcomplex we are interested in finding
    @return state, int, number of times the protein or subcomplex appears
            in subcomplex_components
    """
    state = 0
    for subcomplex in subcomplex_components:
        if prot in subcomplex:
            state += 1
    return state


def composition_likelihood_function(mean, std, prots, node):
    """Function that calculates the likelihood of an individual node, used by
    calc_likelihood().

    @param mean: dictionary of dictionaries where the first key is the protein,
           the second key is the time, and the expected mean copy number
           from experiment is returned.
    @param std: dictionary of dictionaries where the first key is the protein,
           the second key is the time, and the expected standard deviation
           of protein copy number from experiment is returned.
    @param prots: list of proteins or subcomplexes which will be scored
           according to this likelihood function
    @param node: the graphNode object for which the likelihood will be
           calculated.
    @return w: float, the weight of the graphNode according to the composition
            likelihood function.
    """
    # get time
    t = node.get_time()
    w = 0
    for prot in prots:
        # x counts the number of proteins of a given type in the node
        x = get_state(node.get_subcomplex_components(), prot)
        # check std is greater than 0
        if std[prot][t] > 0:
            pass
        else:
            warnings.warn(
                'WARNING!!! Standard deviation of protein ' + prot
                + ' 0 or less at time ' + t
                + '. May lead to illogical results.')
        w += (0.5 * ((x - mean[prot][t]) / std[prot][t])**2
              + np.log(std[prot][t] * np.sqrt(2 * np.pi)))
    return w


def calc_likelihood(exp_comp_map, nodes):
    """
    Function that adds a score for the compositional likelihood for all
    states represented as nodes in the graph. The composition likelihood
    assumes a Gaussian distribution for copy number of each protein or
    subcomplex with means and standard deviatiations derived from experiment.
    Returns the nodes, with the new weights added.

    @param exp_comp_map: dictionary, which describes protein stoicheometery.
           The key describes the protein, which should correspond to names
           within the expected_subcomplexes. Only copy numbers for proteins
           or subcomplexes included in this dictionary will be scored. For
           each of these proteins, a csv file should be provided with protein
           copy number data. The csv file should have 3 columns,
           1) "Time", which matches up to the possible times in the graph,
           2) "mean", the average protein copy number at that time point
           from experiment, and 3) "std", the standard deviation of that
           protein copy number from experiment.
    @param nodes: list of graphNode objects, which have been already been
           initiated with static scores
    @return nodes: editted list of graphNode objects, which now have static
            and composition scores
    """
    import pandas as pd
    # Get list of all all proteins
    prots = list(exp_comp_map.keys())
    # Data is stored as a dictionary of dictionaries. The first dictionary
    # references which protein you are refering to.
    # the 2nd dictionary references which time you are refering to. The return
    # is the mean or standard deviation of the protein copy number
    mean = {}
    std = {}
    # import csv file as pandas data frame
    for prot in prots:
        prot_dict_mean = {}
        prot_dict_std = {}
        if os.path.exists(exp_comp_map[prot]):
            exp = pd.read_csv(exp_comp_map[prot])
        else:
            raise Exception(
                "Error!!! Check exp_comp_map. Unable to find composition "
                "file: " + exp_comp_map[prot] + '\nClosing...')
        for i in range(len(exp)):
            prot_dict_mean[exp['Time'][i]] = exp['mean'][i]
            prot_dict_std[exp['Time'][i]] = exp['std'][i]
        mean[prot] = prot_dict_mean
        std[prot] = prot_dict_std
    # loop over all nodes and calculate the likelihood for each noe
    for node in nodes:
        # compute the compositional likelihood of the nodes
        weight = composition_likelihood_function(mean, std, prots, node)
        # add state weight to node
        node.add_score(float(weight))
    return nodes

def calc_likelihood_state(exp_comp_map, t, state):
    """
    Function that adds a score for the compositional likelihood for all
    states, similar to how composition_likelihood_function calculates the
    composition likelihood of a node. Used by prepare_protein_library.
    The composition likelihood assumes a Gaussian distribution for copy
    number of each protein or subcomplex with means and standard
    deviatiations derived from experiment. Returns the nodes, with the
    new weights added.

    @param exp_comp_map: dictionary, which describes protein stoicheometery.
           The key describes the protein, which should correspond to names
           within the expected_subcomplexes. Only copy numbers for proteins
           or subcomplexes included in this dictionary will be scored. For
           each of these proteins, a csv file should be provided with protein
           copy number data. The csv file should have 3 columns,
           1) "Time", which matches up to the possible times in the graph,
           2) "mean", the average protein copy number at that time point
           from experiment, and 3) "std", the standard deviation of that
           protein copy number from experiment.
    @param t: string, time at which the composition likelihood should be
            calculated. Should match one a possible value in the first column
            of the exp_comp_map.
    @param state: list of integers, an array of the number of protein copy numbers
            for which the likelihood will be calculated. This array should list
            the proteins in the same order as the exp_comp_map.
    @return weight: float, the weight of the graphNode according to the composition
            likelihood function.
    """
    import pandas as pd
    # Data is stored as a dictionary of dictionaries. The first dictionary
    # references which protein you are refering to.
    # the 2nd dictionary references which time you are refering to. The return
    # is the mean or standard deviation of the protein copy number
    mean = {}
    std = {}
    state_cn={}
    count=0
    # import csv file as pandas data frame
    for prot in exp_comp_map.keys():
        prot_dict_mean = {}
        prot_dict_std = {}
        state_cn[prot]=state[count]
        if os.path.exists(exp_comp_map[prot]):
            exp = pd.read_csv(exp_comp_map[prot])
        else:
            raise Exception(
                "Error!!! Check exp_comp_map. Unable to find composition "
                "file: " + exp_comp_map[prot] + '\nClosing...')
        for i in range(len(exp)):
            prot_dict_mean[exp['Time'][i]] = exp['mean'][i]
            prot_dict_std[exp['Time'][i]] = exp['std'][i]
        mean[prot] = prot_dict_mean
        std[prot] = prot_dict_std
        count += 1
    # compute the compositional likelihood of the nodes
    weight=0
    for prot in exp_comp_map.keys():
        # x counts the number of proteins of a given type in the node
        x = state_cn[prot]
        # check std is greater than 0
        if std[prot][t] > 0:
            pass
        else:
            warnings.warn(
                'WARNING!!! Standard deviation of protein ' + prot
                + ' 0 or less at time ' + t
                + '. May lead to illogical results.')
        weight += (0.5 * ((x - mean[prot][t]) / std[prot][t]) ** 2 + np.log(std[prot][t] * np.sqrt(2 * np.pi)))
    return weight