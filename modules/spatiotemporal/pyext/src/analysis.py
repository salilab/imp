import numpy as np
import sys

# Function that reads in two labeled_pdfs from create_DAG and returns the temporal_precision, defined as the probability overlap between two pathway models.
# Inputs: labaled_pdf1_fn - labeled_pdf from one independent sampling; labaled_pdf2_fn - labeled_pdf from another independent sampling
# output_fn - name of output file (default: 'temporal_precision.txt')
# Ouptut: temporal precision, written to output_fn
def temporal_precision(labeled_pdf1_fn,labeled_pdf2_fn,output_fn='temporal_precision.txt'):
    pdf_files=[labeled_pdf1_fn,labeled_pdf2_fn]
    dict_list=[]
    for pdf_file in pdf_files:
        # create blank dictonary to store the results
        prob_dict = {}
        # read in labeled pdf file
        old = open(pdf_file, 'r')
        line = old.readline()
        # store the path through various nodes, as well as the probability of that path
        while line:
            line_split = line.split()
            if len(line_split) == 2:
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
    # precision starts at 1
    precision = 1
    for key in key_list2:
        if key in key_list:
            # reduce by 1/2 the Manhattan distance between probabilities
            precision-=0.5*np.abs(dict_list[0][key]-dict_list[1][key])
        else:
            # states in key_list2, but not key_list inherently contribute to the temporal precision
            precision -= 0.5 * np.abs(dict_list[1][key])
    for key in key_list:
        if key in key_list2:
            pass
        # states in key_list, but not key_list2 inherently contribute to the temporal precision
        else:
            precision -= 0.5 * np.abs(dict_list[0][key])
    new=open(output_fn,'w')
    new.write('Temporal precision between '+labeled_pdf1_fn+' and '+labeled_pdf2_fn+':\n')
    new.write(str(precision))
    print('Temporal precision between '+labeled_pdf1_fn+' and '+labeled_pdf2_fn+':')
    print(precision)

    return