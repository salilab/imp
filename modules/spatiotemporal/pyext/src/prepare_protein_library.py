"""@namespace IMP.spatiotemporal.prepare_protein_library
   Function for preparing spatiotemporal models for sampling.
"""
import numpy as np
import itertools
import pandas as pd
from IMP.spatiotemporal import composition_scoring

def prepare_protein_library(times, exp_comp_map, expected_subcomplexes, nmodels, template_topology='', template_dict={}, match_final_state=True):
    """
        Function that reads in experimental stoicheometery data and calculates which compositions and location
        assignments should be sampled for spatiotemporal modeling, which are saved as config files. Optionally, a PMI
        topology file can be provided, in which case topology files for each composition and location assignment are
        also written.
        The output is 3 types of files:
        1. *_time.config - configuration files, which list the proteins included at each time point for each model
        2. time.txt - protein copy number files. Each row is a protein copy number state and each column is the
            protein copy number in that state. Note that each protein copy number state can result in multiple
            location assignments.
        3. *_time_topol.txt - topology files for each copy number and location assignment.

        @param times: list of strings, the times at which the stoicheometery data should be read.
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
        @param expected_subcomplexes: list of all possible subcomplex strings
           in the model. Should be a list without duplicates of
           all components in the subcomplex configuration files.
        @param nmodels: int, number of models with different protein copy numbers to generate at each time point.
        @param template_topology: string, name of the topology file for the complete complex.
            (default: '', no topology files are output)
        @param template_dict: dictionary for connecting the spatiotemporal model to the topology file.
            The keys (string) are the names of the proteins, defined by the expected_complexes variable.
            The values (list) are the names of all proteins in the topology file that should have the same copy number
             as the labeled protein, specifically the "molecule_name."
            (default: {}, no topology files are output)
        @param match_final_state: Boolean, determines whether to fix the final state to the state defined by
            expected_subcomplexes. True enforces this match and thus ensures that the final time has only one state.
            (default: True)
        """
    # Assert that all inputs are the correct variable type
    if not isinstance(times, list):
        raise TypeError("times should be of type list")
    if not isinstance(exp_comp_map, dict):
        raise TypeError("times should be of type dict")
    if not isinstance(nmodels, int):
        raise TypeError("nmodels should be of type int")
    if not isinstance(template_topology, str):
        raise TypeError("template_topology should be of type str")
    if not isinstance(template_dict, dict):
        raise TypeError("template_dict should be of type dict")
    if not isinstance(match_final_state, bool):
        raise TypeError("match_final_state should be of type bool")
    # Whether or not topology files should be written
    include_topology = False
    # calculate final copy numbers based on the expected complexes
    final_CN=np.zeros(len(exp_comp.keys()),dtype=int)
    for i, key in enumerate(exp_comp.keys()):
        for subcomplex in expected_subcomplexes:
            if key in subcomplex:
                final_CN[i] += 1
    # Enumerate all possible compositions (independent of time)
    # start by computing the range for each protein copy number
    copy_num=[]
    for CN in final_CN:
        copy_num.append(range(CN+1))
    # Convert the range of protein copy numbers to all possible combinations of states
    all_library = [l for l in itertools.product(*copy_num)]
    # remove empty state
    empty_state=[]
    for i in range(len(final_CN)):
        empty_state.append(0)
    all_library.pop(all_library.index(tuple(empty_state)))

    # compare the complete composition library against the composition model
    # at each time and extract top scoring compositions
    for time in times:
        # if match_final_state is True, restrict the final state to match the known copy numbers
        if time==times[len(times)-1] and match_final_state:
            olist=[list(final_CN)]
            state_list=[]
            # Keep all expected complexes
            state_list.append(expected_subcomplexes)
        # otherwise, calculate the nmodels most likely copy numbers for each time point
        else:
            # caclulate the weights for each state
            unnormalized_weights =[]
            for state in all_library:
                unnormalized_weights.append(composition_scoring.calc_likelihood_state(exp_comp_map,time,state))
            unw = np.array(unnormalized_weights)
            print(time)
            print(all_library)
            print(unw)
            # get top scoring nmodels
            mindx = np.argsort(unw)[0:nmodels]
            # write out library with the top scoring models
            olist = []
            state_list = []
            for m in mindx:
                state = all_library[m]
                # convert state counts to protein list
                olist.append(list(state))
            # Loops over each copy number state
            for state in olist:
                cn_list=[]
                # Loops over each protein
                for i, cn in enumerate(state):
                    sub_list=[]
                    # initiate found_subcomplex
                    found_subcomplex = []
                    for subcomplex in expected_subcomplexes:
                        # See which subcomplexes this protein appears in
                        if list(exp_comp_map.keys())[i] in subcomplex:
                            found_subcomplex.append(subcomplex)
                    # find proteins using combinatorics
                    prots=list(itertools.combinations(found_subcomplex, state[i]))
                    for prot in prots:
                        sub_list.append(prot)
                    if len(sub_list[0])>0:
                        cn_list.append(sub_list)
                # combine the list of copy numbers
                all_cn = [l for l in itertools.product(*cn_list)]
                # format the copy number list from a list (all copy number combinations) of tuples (copy number of 1
                # state) of tuples (name of 1 protein) to a list (all copies) of lists (all proteins)
                # loop over all cn
                for cn in all_cn:
                    cn_list2=[]
                    # select each copy number
                    for n in cn:
                        # append each protein in each copy number to list
                        for prot in n:
                            cn_list2.append(prot)
                    state_list.append(cn_list2)
        # write top "scoring" compositions to file
        oary = np.array(olist, dtype=int)
        header=''
        for prot_name in exp_comp.keys():
            header=header+str(prot_name)+'\t\t\t\t'
        np.savetxt( time + ".txt", oary,header=header)

        # write protein config library to file
        for indx,prot_list in enumerate(state_list):
            with open(str(indx+1) + "_" + time + ".config", "w") as fh:
                for prot in prot_list:
                    fh.write(prot +"\n")

        if len(template_topology)>0:
            include_topology=True
            # write topology file for each state
            for indx,prot_list in enumerate(state_list):
                # Names of proteins to keep, according to template file
                keep_prots=[]
                # loop over each entry of prot_list and convert to the template file name
                for prot in prot_list:
                    if prot in template_dict.keys():
                        keep_prots.extend(template_dict[prot])
                    else:
                        raise Exception("Protein " + prot + ' does not exist in template_dict\nClosing...')
                # open new topology file
                with open(str(indx+1) + "_" + time + "_topol.txt", "w") as fh:
                    old=open(template_topology,'r')
                    line=old.readline()
                    while line:
                        line_split = line.split('|')
                        # keep blank lines
                        if len(line_split)<2:
                            fh.write(line)
                        else:
                            # Keep lines where the protein name is in the template_dict
                            if line_split[1] in keep_prots:
                                fh.write(line)
                            # Keep instruction line
                            elif line_split[1] == 'molecule_name ':
                                fh.write(line)
                        line=old.readline()
                    old.close()
                    fh.close()
    if include_topology:
        print('Successfully calculated the most likely configurations, and saved them to configuration and topology '
              'files.')
    else:
        print('Successfully calculated the most likely configurations, and saved them to configuration files.')