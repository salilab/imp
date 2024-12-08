## \example rmf_xml.py
# This example converts an RMF file to xml, in order to display it in
# an XML viewer such as firefox. Its functionality is largely
# identical to the \c rmf_xml program. The secondary purpose it to
# provide an example of extracting all data from an RMF file and
# converting it to some other source. The data extracted here is
# completely uninterpreted (eg, cartesian coordinates are treated just
# the same as other values). See the source code of rmf_xml for a
# similar example in C++.

import RMF

# don't bother with command line arguments, to keep in simple
file_name = RMF.get_example_path("simple.rmf3")
verbose = True

# show the data with the specified key category


def show_data_xml(nh, kc):
    rh = nh.get_file()
    # get all the keys, we could pull this up in the call stack
    keys = rh.get_keys(kc)
    opened = False
    for k in keys:
        v = nh.get_value(k)
        if v is not None:
            if not opened:
                print("<", rh.get_name(kc))
                opened = True
            name = rh.get_name(k)
            name.replace(" ", "_")
            print(name, "=\"" + str(v) + "\"")
    if opened:
        print("/>")


def show_xml(nh, kcs):
    name = nh.get_name()
    name.replace(" ", "_")
    print("<node name=\"" + name + "\" id=\"" + str(nh.get_id().get_index())\
        + "\" type=\"" + str(nh.get_type()) + "\"/>")
    if verbose:
        for kc in kcs:
            show_data_xml(nh, kc)
    children = nh.get_children()
    for c in children:
        print("<child>")
        show_xml(c, kcs)
        print("</child>")

# open the file, and don't clear the contents
rh = RMF.open_rmf_file_read_only(file_name)
rh.set_current_frame(RMF.FrameID(0))
print("<?xml version=\"1.0\"?>")
print("<rmf>")
print("<path>")
print(file_name)
print("</path>")
print("<description>")
print(rh.get_description())
print("</description>")
print("<path>")
print(input)
print("</path>")
kcs = rh.get_categories()
show_xml(rh.get_root_node(), kcs)
print("</rmf>")
