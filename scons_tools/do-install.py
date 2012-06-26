#!/usr/bin/python
import optparse
import glob
import os.path

parser = optparse.OptionParser()
parser.add_option("-i", "--include", dest="include",
                  help="Where to install include files")
parser.add_option("-d", "--data", dest="data",
                  help="Where to install data files")
parser.add_option("-l", "--lib", dest="lib",
                  help="Where to install lib files")
parser.add_option("-p", "--python", dest="python",
                  help="Where to install python files")
parser.add_option("-s", "--swig", dest="swig",
                  help="Where to install swig files")
parser.add_option("-c", "--doc", dest="doc",
                  help="Where to install doc files")
(options, args) = parser.parse_args()

def install_hierarchy(source, dest, include=["*"]):
    print source
    matching=[]
    for p in include:
        matching+=glob.glob(os.path.join(source, p))
    curlist=[]
    for m in matching:
        if os.path.isdir(m):
            install_hierarchy(m, os.path.join(dest, os.path.relpath(m, source)))
        else:
            curlist.append(m)
    print "installing into", dest
    if len(curlist) > 0:
        os.system("install -d "+dest)
        os.system("install -t "+dest + " " + " ".join(curlist))

install_hierarchy("build/include", options.include)
install_hierarchy("build/data", options.data)
install_hierarchy("build/lib", options.lib, include=["lib*"])
install_hierarchy("build/lib", options.python, include=["_*", "IMP", "RMF"])
install_hierarchy("build/swig", options.swig)
install_hierarchy("build/doc", options.doc)
