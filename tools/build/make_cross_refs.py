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
def _cleanup_name(n):
    if n.find("href") != -1:
        return n
    if n.find("::") != -1:
        sp = n.split("::")
        if len(sp) == 2:
            sp = [sp[0], "kernel", sp[1]]
        return "::".join(sp)
    if n == 'IMP_NEW()':
        # Workaround for (unqualified) macro
        return "IMP::base::" + n
    if n.find(".py") != -1 or n.find(".cpp") != -1:
        m = n.split("/")[0]
        return "[%s](%s/%s)"%(n, m, n.replace("/", "_2").replace(".", "_8")+"-example.html")
    else:
        # fix later
        return None
def get_function_name(et):
    name= et.text
    nname= name.replace(".", "::")
    if not nname.startswith("IMP"):
        return None
    return nname

def get_function_link(name, et, mname):
    nicename= name+"::"+et.find(".//name").text
    refid= et.attrib['id']
    split= refid.split("_1")
    fname= "_1".join(split[:-1])+".html"
    tag= split[-1]
    return "<a href=\""+mname + "/" + fname+"#"+tag+"\">"+nicename+"()</a>"

def _add_to_list(table, key, value):
    _cleanup_name(key)
    _cleanup_name(value)
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
    if name: _cleanup_name(name)
    if et.tag=='ref':
        if et.attrib['kindref'] == "compound":
            _add_example_class_ref(name, get_class_name(et))
        if et.attrib['kindref'] == "member":
            _add_example_function_ref(name, get_function_name(et))
    for child in et:
        traverse_example(name, child)

def traverse_example_2(name, et):
    if name: _cleanup_name(name)
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

def traverse_class(name, et, module):
    if et.tag in ['listofallmembers', 'collaborationgraph', 'inheritancegraph']:
        return
    if et.tag == 'memberdef' and et.attrib["kind"]=="function":
        membername=get_function_link(name, et, module)
        traverse_ret(membername, et.find(".//type"))
        for p in et.findall(".//param"):
            traverse_param(membername, p)
    else:
        for child in et:
            traverse_class(name, child, module)

def get_namespace_name(et):
    return et.find(".//compoundname").text

def traverse_namespace(name, et, module):
    if et.tag == 'memberdef' and et.attrib["kind"]=="function":
        membername=get_function_link(name, et, module)
        traverse_ret(membername, et.find(".//type"))
        for p in et.findall(".//param"):
            traverse_param(membername, p)
    else:
        for child in et:
            traverse_namespace(name, child, module)


def create_index(title, ref, other_indexes, description, links, target, key_name, target_name):
    out= open(target, "w")
    out.write("# %s\n"%title)
    out.write("# Overview # {#%s}\n"%ref)
    out.write("[TOC]\n");
    out.write(description+"\n\n")
    out.write("See also "+", ".join(["[%s](@ref %s)"%(x[0], x[1]) for x in other_indexes]) + "\n")
    keys=links.keys()
    keys.sort()
    keys_by_module = {}
    for k in keys:
        m = _cleanup_name(k).split("::")[1]
        if m not in keys_by_module.keys():
            keys_by_module[m]=[]
        keys_by_module[m].append(k)
    modules = keys_by_module.keys()
    modules.sort()
    for m in modules:
        out.write("# IMP.%s # {#%s_%s}\n"%(m, ref, m))
        out.write("<table><tr>\n")
        out.write("<th>%s</th><th>%s</th></tr>\n"%(key_name, target_name))
        for k in keys_by_module[m]:

            cn = _cleanup_name(k)
            out.write("<tr><td>@ref %s</td>"% cn)
        # suppress same names as they aren't very useful
            seen = []
            entry = []
            for l in links[k]:
                cn = _cleanup_name(l)
                if cn and cn not in seen:
                    entry.append(cn)
                seen.append(cn)
            out.write("<td>%s</td></tr>\n"%", ".join(entry))
        out.write("</table>\n")

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
        module = os.path.split(os.path.split(os.path.split(f)[0])[0])[1]
        try:
            et= ET.parse(f)
        except ET.ParseError:
            print >> sys.stderr, "ERROR parsing", f
        fname=os.path.basename(f)
        if fname.startswith("namespaceIMP"):
            if verbose:
                print "namespace", fname
            traverse_namespace(get_namespace_name(et.getroot()), et.getroot(), module)
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
            traverse_class(get_file_class_name(et.getroot()), et.getroot(), module)
        else:
            if verbose:
                print "skipping", fname
    indexes = [("Factory Index", "factory_index"),
               ("Argument Index", "argument_index"),
               ("Class Examples", "class_example_index"),
               ("Function Examples", "function_example_index")]
    create_index(indexes[0][0], indexes[0][1], indexes[1:], "Functions that create objects of a given type:",
                 creates, "doxygen/generated/factory_index.md", "Class", "Factories")
    create_index(indexes[1][0], indexes[1][1], indexes, "Functions that take objects of a given type as arguments:",
                 takes, "doxygen/generated/argument_index.md", "Class", "Users")
    create_index(indexes[2][0], indexes[2][1], indexes, "Examples that use a given class:",
                 examples_classes, "doxygen/generated/class_example_index.md", "Class", "Examples")
    create_index(indexes[3][0], indexes[3][1], indexes[:-1], "Examples that use a given function:",
                 examples_functions, "doxygen/generated/function_example_index.md", "Function", "Examples")

if __name__ == '__main__':
    main()
