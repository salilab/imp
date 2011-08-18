import IMP.rmf

# don't bother with command line arguments, to keep in simple
file_name=IMP.rmf.get_example_path("fragment.rmf")
frame=0
verbose=True

# show the data with the specified key category
def show_data_xml(nh, kc):
    rh= nh.get_root_handle()
    # get all the keys, we could pull this up in the call stack
    keys= rh.get_keys(kc)
    opened=False
    for k in keys:
        if nh.get_has_value(k, frame):
            if not opened:
                print "<", kc
                opened=True
            name=rh.get_name(k)
            name.replace(" ", "_")
            print name,"=\""+str(nh.get_value(k, frame))+"\""
    if opened:
        print "/>"

def show_xml(nh, kcs):
    name=nh.get_name()
    name.replace(" ", "_")
    print "<node name=\""+name+"\" id=\""+str(nh.get_id())+"\" type=\""+IMP.rmf.get_type_name(nh.get_type())+"\"/>";
    if verbose:
        for kc in kcs:
            show_data_xml(nh, kc)
    children= nh.get_children()
    for c in children:
        print "<child>"
        show_xml(c, kcs);
        print "</child>"

# open the file, and don't clear the contents
rh= IMP.rmf.open_rmf_file(file_name);
print "<?xml version=\"1.0\"?>"
print "<rmf>"
print "<description>"
print rh.get_description()
print "</description>"
print "<path>"
print input
print "</path>"
kcs= rh.get_categories()
show_xml(rh, kcs)
if rh.get_number_of_bonds() >0:
    print "<bonds>";
    for b in rh.get_bonds():
        print "<bond id0=\""+str(b[0].get_id())+"\" id1=\""+str(b[1].get_id())+"\"/>"
    print "</bonds>"
print "</rmf>";
