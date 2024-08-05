"""@namespace IMP.spatiotemporal.analysis
   Functions to analyze spatiotemporal models.
"""
import numpy as np


def temporal_precision(labeled_pdf1_fn, labeled_pdf2_fn,
                       output_fn='temporal_precision.txt'):
    """
    Function that reads in two labeled_pdfs from create_DAG and returns the
    temporal_precision, defined as the probability overlap between two
    pathway models.

    @param labeled_pdf1_fn: string, labeled pdf file name (including the path);
           labeled_pdf from one independent sampling
    @param labeled_pdf2_fn: string, labeled pdf file name (including the path);
           labeled_pdf from another independent sampling
    @param output_fn: string, name of output file
           (default: 'temporal_precision.txt')
    @return temporal precision, written to output_fn
    """
    pdf_files = [labeled_pdf1_fn, labeled_pdf2_fn]
    dict_list = []
    for pdf_file in pdf_files:
        # create blank dictonary to store the results
        prob_dict = {}
        # read in labeled pdf file
        old = open(pdf_file, 'r')
        line = old.readline()
        # store the path through various nodes, as well as the probability
        # of that path
        while line:
            line_split = line.split()
            # assumes the first string is the trajectory string, the second
            # string is the probability
            if len(line_split) > 1:
                # use # for comments
                if line_split[0] == '#':
                    pass
                else:
                    trj = line_split[0]
                    prob = float(line_split[1])
                    # store in dictionary
                    prob_dict[trj] = prob
            line = old.readline()
        old.close()
        # append dictionary to dict_list
        dict_list.append(prob_dict)
    # calculate
    key_list = dict_list[0].keys()
    key_list2 = dict_list[1].keys()
    # print error if keys not found
    if len(key_list) == 0 or len(key_list2) == 0:
        raise Exception('Error reading labeled_pdf!!! Keys not found')
    # precision starts at 1
    precision = 1
    for key in key_list2:
        if key in key_list:
            # reduce by 1/2 the Manhattan distance between probabilities
            precision -= 0.5 * np.abs(dict_list[0][key] - dict_list[1][key])
        else:
            # states in key_list2, but not key_list inherently contribute
            # to the temporal precision
            precision -= 0.5 * np.abs(dict_list[1][key])
    for key in key_list:
        if key in key_list2:
            pass
        # states in key_list, but not key_list2 inherently contribute to
        # the temporal precision
        else:
            precision -= 0.5 * np.abs(dict_list[0][key])
    with open(output_fn, 'w') as new:
        new.write('Temporal precision between ' + labeled_pdf1_fn + ' and '
                  + labeled_pdf2_fn + ':\n')
        new.write(str(precision))
    print('Temporal precision between ' + labeled_pdf1_fn + ' and '
          + labeled_pdf2_fn + ':')
    print(precision)


def precision(labeled_pdf_fn, output_fn='precision.txt'):
    """
    Function that reads in one labeled_pdf from create_DAG and returns the
    precision, defined as the sum of the squared
    probability of all trajectories.

    @param labeled_pdf_fn: string, labeled pdf file name (including the path);
           labeled_pdf from the total model
    @param output_fn: string, name of output file
           (default: 'temporal_precision.txt')
    @return temporal precision, written to output_fn
    """
    # create blank dictonary to store the results
    prob_list = []
    # read in labeled pdf file
    old = open(labeled_pdf_fn, 'r')
    line = old.readline()
    # store the path through various nodes, as well as the probability
    # of that path
    while line:
        line_split = line.split()
        # assumes the first string is the trajectory string, the second
        # string is the probability
        if len(line_split) > 1:
            # use # for comments
            if line_split[0] == '#':
                pass
            else:
                prob = float(line_split[1])
                # store in dictionary
                prob_list.append(prob)
            line = old.readline()
    old.close()
    pure = 0
    for prob in prob_list:
        pure += prob * prob
    with open(output_fn, 'w') as new:
        new.write('Precision of ' + labeled_pdf_fn + ':\n')
        new.write(str(pure))
    print('Precision of ' + labeled_pdf_fn)
    print(str(pure))
