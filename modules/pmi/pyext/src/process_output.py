#! /usr/bin/env python

from __future__ import print_function
import argparse
import difflib


p = argparse.ArgumentParser(
            description="Process output data file saved as dictionaries. "
                        "It has two modalities: print selected fields for all "
                        "lines or print a particular line where a field has a "
                        "given value. Example of usage: process_output.py "
                        "--soft -s To E S -f log.3.native-2-no-red. "
                        "process_output.py --soft --search_field EV0 "
                        "--search_value 5.67750116023 "
                        "-f log.3.native-2-no-red")
p.add_argument('-f', action="store", dest="filename",
               help="file name to process")
p.add_argument('-s', dest="fields", nargs="+",
               help="Specify all fields to be printed. Multiple flags "
                    "will append a list of fields to be printed")
p.add_argument('-t', dest="single_column_field",
               help="Specify a single column field to be printed. It "
                    "will be printed as a column. If the field name is "
                    "not complete, it will print all fields whose name "
                    "contain the queried string.")
p.add_argument('-p', action="store_true", dest="print_fields",
               default=False, help="print the fields contained in the file")
p.add_argument('--head', action="store_true", dest="print_header",
               default=False,
               help="print the fields contained in the file (only stat2)")
p.add_argument('-n', action="store", dest="print_raw_number",
               help="print the selected raw")
p.add_argument('--soft', action="store_true", dest="soft_match", default=False,
               help="Soft match. Closest matching field will be printed, "
                    "e.g. S will give Step_Number, En will give energy, etc. ")
p.add_argument('--search_field', dest="search_field",
               help="Search a line from the file. Specify the field to "
                    "be searched for. ")
p.add_argument('--search_value', dest="search_value",
               help="Search a line from the file. Specify the value to "
                    "be searched for. ")
p.add_argument('--nframe', action="store_true", dest="nframe", default=False,
               help="Print the frame number as initial column")

result = p.parse_args()

isstat1 = False
isstat2 = False

# open the file
if result.filename is not None:
    f = open(result.filename, "r")
else:
    raise ValueError("No file name provided. Use -h for help")

# get the keys from the first line
for line in f.readlines():
    d = eval(line)
    klist = list(d.keys())
    # check if it is a stat2 file
    if "STAT2HEADER" in klist:
        import operator
        isstat2 = True
        for k in klist:
            if "STAT2HEADER" in str(k):
                if result.print_header:
                    print(k, d[k])
                del d[k]
        stat2_dict = d
        # get the list of keys sorted by value
        kkeys = [k[0]
                 for k in sorted(stat2_dict.items(),
                                 key=operator.itemgetter(1))]
        klist = [k[1]
                 for k in sorted(stat2_dict.items(),
                                 key=operator.itemgetter(1))]
        invstat2_dict = {}
        for k in kkeys:
            invstat2_dict.update({stat2_dict[k]: k})
    else:
        isstat1 = True
        klist.sort()

    break
f.close()

# print the keys
if result.print_fields:
    for key in klist:
        if len(key) <= 100:
            print(key)
        else:
            print(key[0:100],
                  "... omitting the rest of the string (>100 characters)")


# the field string matching is by default strict, i.e., the input string
# must be the same as the one in the file
match_strictness = 1.0
if result.soft_match:
    match_strictness = 0.1

# print the queried fields
if result.fields is not None:
    field_list = []
    # check whether the fields exist and convert them to best matching existing
    # field names
    for field in result.fields:
        found_entries = difflib.get_close_matches(
            field,
            klist,
            1,
            match_strictness)
        if len(found_entries) == 0:
            raise ValueError("field " + field + " non found")
        else:
            field_list.append(found_entries[0])

    # print comment line
    s0 = ' '.join(["%20s" % (field) for field in field_list])
    print("# " + s0)

    # print fields values
    f = open(result.filename, "r")
    line_number = 0
    for line in f.readlines():
        line_number += 1
        try:
            d = eval(line)
        except:  # noqa: E722
            print("# Warning: skipped line number " +
                  str(line_number) + " not a valid line")
            continue
        if isstat1:
            s0 = ' '.join(["%20s" % (str(d[field])) for field in field_list])
        elif isstat2:
            if line_number == 1:
                continue
            s0 = ' '.join(["%20s" % (str(d[invstat2_dict[field]]))
                          for field in field_list])
        if not result.nframe:
            print("> " + s0)
        else:
            print(str(line_number) + " > " + s0)
    f.close()


if result.single_column_field is not None:
    field_list = []
    for k in klist:
        if result.single_column_field in k:
            field_list.append(k)

    f = open(result.filename, "r")
    line_number = 0
    for line in f.readlines():
        line_number += 1
        try:
            d = eval(line)
        except:  # noqa: E722
            print("# Warning: skipped line number " +
                  str(line_number) + " not a valid line")
            continue
        if isstat1:
            for key in field_list:
                print(key, d[key])
        elif isstat2:
            if line_number == 1:
                continue
            for key in field_list:
                print(key, d[invstat2_dict[key]])
        print(" ")
    f.close()

if (result.search_field is not None) and (result.search_value is not None):
    # check whether the fields exist and convert them to best matching existing
    # field names
    found_entries = difflib.get_close_matches(
        result.search_field,
        klist,
        1,
        match_strictness)
    if len(found_entries) == 0:
        raise ValueError("field " + result.search_field + " non found")
    else:
        corrected_field = found_entries[0]
    # print fields values
    f = open(result.filename, "r")
    line_number = 0
    for line in f.readlines():
        line_number += 1
        try:
            d = eval(line)
        except:  # noqa: E722
            print("# Warning: skipped line number " +
                  str(line_number) + " not a valid line")
            continue

        if isstat1:
            if (str(d[corrected_field]) == result.search_value):
                for key in klist:
                    print(key, d[key])
        elif isstat2:
            if line_number == 1:
                continue
            if (str(d[invstat2_dict[corrected_field]]) == result.search_value):
                for key in klist:
                    print(key, d[invstat2_dict[key]])
    f.close()

if result.print_raw_number is not None:
    # check whether the fields exist and convert them to best matching existing
    # field names
    f = open(result.filename, "r")
    line_number = 0
    for line in f.readlines():
        line_number += 1
        if isstat1:
            if (line_number == int(result.print_raw_number)):
                try:
                    d = eval(line)
                except:  # noqa: E722
                    print("# Warning: skipped line number "
                          + str(line_number) + " not a valid line")
                    break
                for key in klist:
                    print(key, d[key])

        elif isstat2:
            if (line_number == int(result.print_raw_number) + 1):
                try:
                    d = eval(line)
                except:  # noqa: E722
                    print("# Warning: skipped line number "
                          + str(line_number) + " not a valid line")
                    break
                for key in klist:
                    print(key, d[invstat2_dict[key]])
    f.close()
