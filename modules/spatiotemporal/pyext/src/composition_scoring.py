import numpy as np
import pandas as pd
import os

# function to calculate how many times a protein appears in a list of proteins, which can be accessed from a graphNode object using node.get_subcomplex_components()
def get_state(subcomplex_components,prot):
    state=0
    for subcomplex in subcomplex_components:
        if prot in subcomplex:
            state += 1
    return state


def composition_likelihood_function(mean, std, prots, node):
    """Return the weight for a given time and sigma.
    """
    # get time
    t=node.get_time()
    w=0
    for prot in prots:
        # x counts the number of proteins of a given type in the node
        x=get_state(node.get_subcomplex_components(),prot)
        # check std is greater than 0
        if std[prot][t]>0:
            pass
        else:
            print('WARNING!!! Standard deviation of protein '+prot+' 0 or less at time '+t+'. May lead to illogical results.')
        w+=0.5*((x - mean[prot][t]) / std[prot][t])**2 + np.log(std[prot][t]*np.sqrt(2*np.pi))
    return w

def calc_likelihood(exp_comp_map,nodes):
    # Get list of all all proteins
    prots=list(exp_comp_map.keys())
    # Data is stored as a dictionary of dictionaries. The first dictionary references which protein you are refering to.
    # the 2nd dictionary references which time you are refering to. The return is the mean or standard deviation of the protein copy number
    mean={}
    std={}
    # import csv file as pandas data frame
    for prot in prots:
        prot_dict_mean={}
        prot_dict_std={}
        if os.path.exists(exp_comp_map[prot]):
            exp=pd.read_csv(exp_comp_map[prot])
        else:
            print("Error!!! Check exp_comp_map. Unable to find composition file: " + exp_comp_map[prot] + '\nClosing...')
            exit()
        for i in range(len(exp)):
            prot_dict_mean[exp['Time'][i]]=exp['mean'][i]
            prot_dict_std[exp['Time'][i]]=exp['std'][i]
        mean[prot]=prot_dict_mean
        std[prot]=prot_dict_std
    # loop over all nodes and calculate the likelihood for each noe
    for node in nodes:
        # compute the compositional likelihood of the nodes
        weight = composition_likelihood_function(mean, std, prots, node)
        # add state weight to node
        node.add_score(weight)
    return nodes