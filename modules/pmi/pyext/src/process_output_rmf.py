#! /usr/bin/env python


from __future__ import print_function
import argparse

import IMP
import IMP.rmf
import RMF
import difflib


parser = argparse.ArgumentParser(
    description='Process output data file saved as dictionaries. It has two modality: print selected fields for all lines or print a particular line where a filed has a given value. Example of usage: process_output.py --soft -s To E S -f log.3.native-2-no-red. process_output.py --soft --search_field EV0 --search_value 5.67750116023 -f log.3.native-2-no-red')
parser.add_argument(
    '-f',
    action="store",
    dest="filename",
    help="file name to process")
parser.add_argument(
    '-s',
    dest="fields",
    nargs="+",
    help="Specify all fields to be printed. Multiple flags will append a list of fields to be printed")
parser.add_argument(
    '-p',
    action="store_true",
    dest="print_fields",
    default=False,
    help="print the fields contained in the file")
parser.add_argument(
    '-n',
    action="store",
    dest="nframe",
    default=None,
    help="Print frame number n")

result = parser.parse_args()


# open the file
if not result.filename is None:
    try:
        #let's see if that is an rmf file
        rh = RMF.open_rmf_file_read_only(result.filename)
        cat=rh.get_category('stat')
        rmf_klist=rh.get_keys(cat)
        rmf_names_keys=dict([(rh.get_name(k),k) for k in rmf_klist])
        klist=rmf_names_keys.keys()
        del rh
    except IOError:
        raise IOError("Not an RMF file")
else:
    raise ValueError("No file name provided. Use -h for help")


# print the keys
if result.print_fields:
    for key in klist:
        print(key)


# the field string matching is by default strict, i.e., the input string
# must be the same as the one in the file
match_strictness = 1.0

# print the queried fields
if not result.fields is None:
    field_list = []
    # check whether the fields exist and convert them to best maching existing
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
    rh = RMF.open_rmf_file_read_only(result.filename)

    for frame_number in range(rh.get_number_of_frames()):
        IMP.rmf.load_frame(rh, RMF.FrameID(frame_number))
        for name in klist:
            s0=' '.join(["%20s" % (str(rh.get_root_node().get_value(rmf_names_keys[name])))
                          for name in field_list])

        if not result.nframe:
            print("> " + s0)
        else:
            print(str(frame_number)+ " > " + s0)
    del rh

# print given frame number
if not result.nframe is None:
    # print comment line
    # print fields values
    rh = RMF.open_rmf_file_read_only(result.filename)
    IMP.rmf.load_frame(rh, RMF.FrameID(int(result.nframe)))
    for k in klist:
        s0=str(k)+" "+str(rh.get_root_node().get_value(rmf_names_keys[k]))
        print(s0)
    del rh
