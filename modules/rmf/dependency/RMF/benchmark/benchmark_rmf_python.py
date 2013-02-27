#!/usr/bin/env python
import IMP.rmf
import RMF
import IMP.benchmark
import datetime

# don't bother with command line arguments, to keep in simple
file_name=IMP.benchmark.get_data_path("rnapii.rmf")
frame=0
verbose=True

# show the data with the specified key category
def show_data_xml(nh, kc):
    rh= nh.get_root_handle()
    # get all the keys, we could pull this up in the call stack
    keys= rh.get_keys(kc)
    count=0
    for k in keys:
        if nh.get_has_value(k, frame):
            count=count+1
            nh.get_value(k, frame)
    return count
def show_xml(nh, kcs):
    name=nh.get_name()
    name.replace(" ", "_")
    count=1
    nh.get_id()
    IMP.rmf.get_type_name(nh.get_type())
    for kc in kcs:
        count=count+show_data_xml(nh, kc)
    children= nh.get_children()
    for c in children:
        count=count+show_xml(c, kcs);
    return count

start=datetime.datetime.now()
# open the file, and don't clear the contents
rh= RMF.open_rmf_file(file_name);
count= len( rh.get_description())
kcs= rh.get_categories()
count=count+show_xml(rh, kcs)
if rh.get_number_of_bonds() >0:
    for b in rh.get_bonds():
        count=count+1

elapsed= datetime.datetime.now()-start

IMP.benchmark.report("rmf", "python", elapsed.total_seconds(), 0)
