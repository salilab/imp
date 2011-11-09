import RMF

# don't bother with command line arguments, to keep in simple
file_name=RMF.get_example_path("fragment.rmf")
frame=0
verbose=True

# show the data with the specified key category
def show_data_xml(nh, kc, arity=1):
    rh= nh.get_root_handle()
    # get all the keys, we could pull this up in the call stack
    keys= rh.get_keys(kc, arity)
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
    print "<node name=\""+name+"\" id=\""+str(nh.get_id().get_index())\
        +"\" type=\""+RMF.get_type_name(nh.get_type())+"\"/>";
    if verbose:
        for kc in kcs:
            show_data_xml(nh, kc)
    children= nh.get_children()
    for c in children:
        print "<child>"
        show_xml(c, kcs);
        print "</child>"

# open the file, and don't clear the contents
rh= RMF.open_rmf_file(file_name);
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
for i in range(2,5):
    tuples= rh.get_node_tuples(i)
    if len(tuples) >0:
        print "<"+str(i)+"_tuples>"
        for t in tuples:
            print "<"+str(i)+"_tuple id=\""+str(t.get_id().get_index())+"\" type=\""\
                + RMF.get_tuple_type_name(t.get_type())+"\" members=\""\
                +",".join([str(t.get_node(x).get_id().get_index()) for x in range(0, t.get_arity())])+"\"/>"
            if verbose:
                for kc in kcs:
                    show_data_xml(t, kc, i)
        print "</"+str(i)+"_tuples>"
print "</rmf>";
