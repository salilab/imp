"""@namespace IMP.EMageFit.csv_related
   Utility functions to handle CSV files.
"""

import csv


def is_comment(list_values, comment_char="#"):
    if(len(list_values) == 0 or list_values[0][0] == comment_char):
        return True
    return False


def read_csv(fn, delimiter="|", comment_char="#", max_number=False):
    """
        Simple reader of csv files that disregards lines with comments
    """
    f = open(fn, "r")
    reader = csv.reader(f, delimiter=delimiter)
    if(max_number):
        i = 1
        rows = []
        for r in reader:
            if(i > max_number):
                break
            rows.append(r)
            i += 1
    else:
        rows = [d for d in reader if(not is_comment(d, comment_char))]
    f.close()
    return rows


def read_csv_keyword(fn_or_f, keyword, delimiter="|",
                     comment_char="#"):
    """
        Reader of csv files that only recovers lines starting with a keyword

    """
    if(isinstance(fn_or_f, str)):
        f = open(fn_or_f, "r")
    elif(isinstance(fn_or_f, file)):
        f = fn_or_f

    reader = csv.reader(f, delimiter=delimiter)
    rows = []
    for row in reader:
        if(not is_comment(row) and row[0] == keyword):
            rows.append(row)

    if(isinstance(fn_or_f, str)):
        f.close()

    return rows
