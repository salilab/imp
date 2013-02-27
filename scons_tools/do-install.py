#!/usr/bin/env python
import optparse
import glob
import os.path
import shutil

parser = optparse.OptionParser()
parser.add_option("-i", "--include", dest="include",
                  help="Where to install include files")
parser.add_option("-d", "--data", dest="data",
                  help="Where to install data files")
parser.add_option("-l", "--lib", dest="lib",
                  help="Where to install lib files")
parser.add_option("-b", "--bin", dest="bin",
                  help="Where to install binary files")
parser.add_option("-p", "--python", dest="python",
                  help="Where to install python files")
parser.add_option("-s", "--swig", dest="swig",
                  help="Where to install swig files")
parser.add_option("-c", "--doc", dest="doc",
                  help="Where to install doc files")
parser.add_option("--destdir", dest="destdir",
                  help="Directory to prefix to all install paths")
(options, args) = parser.parse_args()

def install_hierarchy(source, dest, include=["*"]):
    print source
    matching=[]
    for p in include:
        matching+=glob.glob(os.path.join(source, p))
    curlist=[]
    for m in matching:
        if os.path.isdir(m):
            install_hierarchy(m, os.path.join(dest, m[len(source)+1:]))
        else:
            curlist.append(m)
    print "installing into", dest
    if len(curlist) > 0:
        if not os.path.exists(dest):
            os.makedirs(dest)
        for f in curlist:
            shutil.copy(f, dest)

install_hierarchy("build/include", options.destdir + options.include)
install_hierarchy("build/data", options.destdir + options.data)
install_hierarchy("build/lib", options.destdir + options.lib,
                  include=["lib*", 'RMF.dll'])
install_hierarchy("build/bin", options.destdir + options.bin)
install_hierarchy("build/lib", options.destdir + options.python,
                  include=["_*", "IMP", "RMF.py"])
install_hierarchy("build/swig", options.destdir + options.swig)
install_hierarchy("build/doc", options.destdir + options.doc)
