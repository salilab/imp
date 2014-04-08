"""@namespace IMP.em2d.argminmax
   Utility functions to extract min/max from the inputs.
"""


def argmin(sequence):
    """ Argmin function: Returns the pair (min_value,min_index),
        where min_index is the index of the minimum value
    """
    min_value = sequence[0]
    min_index = 0
    for i, s in enumerate(sequence):
        if(s < min_value):
            min_value = s
            min_index = i
    return min_value, min_index


def keymin(dictionary):
    """ return the key of the dictionary that has the minimum value """
    ks = dictionary.keys()
    min_key = ks[0]
    min_value = dictionary[min_key]
    for k in ks:
        if(dictionary[k] < min_value):
            min_value = dictionary[k]
            min_key = k
    return min_value, min_key


def argmax(sequence):
    """ Argmax function: Returns the pair (max_value,max_index),
        where max_index is the index of the maximum value
    """
    max_value = sequence[0]
    max_index = 0
    for i, s in enumerate(sequence):
        if(s > max_value):
            max_value = s
            max_index = i
    return max_value, max_index


def keymax(dictionary):
    """ return the key of the dictionary that has the maximum value """
    ks = dictionary.keys()
    max_key = ks[0]
    min_value = dictionary[max_key]
    for k in ks:
        if(dictionary[k] > min_value):
            min_value = dictionary[k]
            max_key = k
    return min_value, max_key
