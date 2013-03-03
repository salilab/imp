#!/usr/bin/env python

""" Parse the doxygen xml files to create cross-ref files that will be read by
doxygen to build web pages. These currently link examples to classes and methods
to classes.
"""

import xml.etree.ElementTree as ET
import tools
import os.path
import sys

creates={}
examples_classes={}
examples_functions={}
takes={}

verbose=False

def get_class_name(et):
    name= et.attrib['refid']
    nname= name[5:].replace("_1", ":").replace("__", "_")
    if not nname.startswith("IMP"):
        return None
    return nname

def get_function_name(et):
    name= et.text
    nname= name.replace(".", "::")
    if not nname.startswith("IMP"):
        return None
    return nname

def get_function_link(name, et):
    nicename= name+"::"+et.find(".//name").text
    refid= et.attrib['id']
    split= refid.split("_1")
    fname= "_1".join(split[:-1])+".html"
    tag= split[-1]
    return "<a href=\""+fname+"#"+tag+"\">"+nicename+"()</a>"

def _add_to_list(table, key, value):
    if table.has_key(key):
        if value not in table[key]:
            table[key].append(value)
    else:
        table[key]=[value]

def _add_example_class_ref(example_name, class_name):
    if not class_name:
        return
    _add_to_list(examples_classes, class_name, example_name)
def _add_example_function_ref(example_name, function_name):
    if not function_name:
        return
    _add_to_list(examples_functions, function_name+"()", example_name)

def _add_takes_ref(name, class_name):
    if not class_name:
        return
    _add_to_list(takes, class_name, name)
def _add_creates_ref(name, class_name):
    if not class_name:
        return
    _add_to_list(creates, class_name, name)

def get_example_name(et):
    nm= et.find('.//location').attrib['file']
    st=os.path.join("build", "doc", "examples")
    rnm= nm[nm.find(st)+len(st)+1:]
    return rnm

def get_example_2_name(et):
    nm= et.find('.//compoundname').text
    #st=os.path.join("build", "doc", "examples")
    #rnm= nm[nm.find(st)+len(st)+1:]
    #return rnm
    return nm

def traverse_example(name, et):
    if et.tag=='ref':
        if et.attrib['kindref'] == "compound":
            _add_example_class_ref(name, get_class_name(et))
        if et.attrib['kindref'] == "member":
            _add_example_function_ref(name, get_function_name(et))
    for child in et:
        traverse_example(name, child)

def traverse_example_2(name, et):
    if et.tag=='ref':
        if et.attrib['kindref'] == "compound":
            _add_example_class_ref(name, get_class_name(et))
        if et.attrib['kindref'] == "member":
            _add_example_function_ref(name, get_function_name(et))
    for child in et:
        traverse_example(name, child)

def get_file_class_name(et):
    nm= et.find('.//compoundname').text
    return nm

def traverse_ret(name, et):
    if et.tag == 'ref':
        if et.attrib['kindref'] == "compound":
            _add_creates_ref(name, get_class_name(et))
    for child in et:
        traverse_ret(name, child)

def traverse_param(name, et):
    if et.tag == 'ref':
        if et.attrib['kindref'] == "compound":
            _add_takes_ref(name, get_class_name(et))
    for child in et:
        traverse_param(name, child)

def traverse_class(name, et):
    if et.tag in ['listofallmembers', 'collaborationgraph', 'inheritancegraph']:
        return
    if et.tag == 'memberdef' and et.attrib["kind"]=="function":
        membername=get_function_link(name, et)
        traverse_ret(membername, et.find(".//type"))
        for p in et.findall(".//param"):
            traverse_param(membername, p)
    else:
        for child in et:
            traverse_class(name, child)

def get_namespace_name(et):
    return et.find(".//compoundname").text

def traverse_namespace(name, et):
    if et.tag == 'memberdef' and et.attrib["kind"]=="function":
        membername=get_function_link(name, et)
        traverse_ret(membername, et.find(".//type"))
        for p in et.findall(".//param"):
            traverse_param(membername, p)
    else:
        for child in et:
            traverse_namespace(name, child)


def create_index(title, ref, description, links, target):
    out= open(target, "w")
    out.write("/** \\page %s %s\n" %(ref, title))
    out.write(description+"\n")
    keys=links.keys()
    keys.sort()
    for k in keys:
        out.write("- %s:\n"%k)
        for l in links[k]:
            out.write("  - %s\n"%l)
    out.write("*/\n")

def main():
    # glob.glob(os.path.join("build", "doxygen", "xml", "*.xml")):
    if len(sys.argv) > 1:
        files= sys.argv[1:]
    else:
        files=tools.get_glob([os.path.join("doxygen", "xml", "*.xml")])
    for f in files:
    #for f in ["doxygen/xml/classIMP_1_1atom_1_1LennardJones.xml"]:
        #["doxygen/xml/namespacetiny.xml",
        #        "doxygen/xml/classIMP_1_1display_1_1Color.xml"]:
        try:
            et= ET.parse(f)
        except ET.ParseError as e:
            print >> sys.stderr, "ERROR parsing", f, e
        fname=os.path.basename(f)
        if fname.startswith("namespaceIMP"):
            if verbose:
                print "namespace", fname
            traverse_namespace(get_namespace_name(et.getroot()), et.getroot())
            #elif fname.startswith("namespace"):
            #if verbose:
            #    print "example 1", fname
            #traverse_example(get_example_name(et.getroot()), et.getroot())
        elif fname.endswith("example.xml"):
            if verbose:
                print "example 2", fname
            traverse_example_2(get_example_2_name(et.getroot()), et.getroot())
        # skip structs for nwo
        elif fname.startswith("classIMP"):
            if verbose:
                print "class", fname
            traverse_class(get_file_class_name(et.getroot()), et.getroot())
        else:
            if verbose:
                print "skipping", fname
    create_index("Factory Index", "factory_index", "Functions that create objects of a given type:",
                 creates, "doxygen/factory_index.dox")
    create_index("Argument Index", "argument_index", "Functions that take objects of a given type as arguments:",
                 takes, "doxygen/argument_index.dox")
    create_index("Class Examples", "class_example_index", "Examples that use a given class:",
                 examples_classes, "doxygen/class_example_index.dox")
    create_index("Function Examples", "function_example_index", "Examples that use a given function:",
                 examples_functions, "doxygen/function_example_index.dox")

if __name__ == '__main__':
    main()
