#!/usr/bin/python
import _tools
from optparse import OptionParser
import os.path
import os
import platform
import stat

template = """#!/bin/sh

@LDPATH@

@PYTHONPATH@

# Where to find data for the various modules
@MODULEROOTS@

# Extra places to look for imp modules
@MODULEPATH@

# Location of binaries (for wine builds, which don't get PATH)
@IMP_BIN_DIR@

@PATH@

@PRECOMMAND@

@TMPDIR@

mkdir -p ${IMP_TMP_DIR}

exec ${precommand} "$@"
"""

parser = OptionParser()
parser.add_option("-p", "--python_path", dest="python_path", default="",
                  help="PYTHONPATH.")
parser.add_option("-l", "--ld_path", dest="ld_path", default="",
                  help="LD_LIB_PATH.")
parser.add_option("-c", "--precommand", dest="precommand", default="",
                  help="Command to run before all executables.")
parser.add_option("-m", "--modules", dest="modules", default="",
                  help="The list of modules.")
parser.add_option("-P", "--path", dest="path", default="",
                  help="The PATH.")
parser.add_option("-x", "--external_modules", dest="external_modules", default="",
                  help="External modules.")
parser.add_option("-d", "--external_data", dest="external_data", default="",
                  help="External data.")
parser.add_option("-B", "--base_dir", dest="base_dir", default=".",
                  help="Base dir, either . or build.")
parser.add_option("-W", "--wine_hack", dest="wine_hack", default="no",
                  help="Base dir, either . or build.")

def main():
    (options, args) = parser.parse_args()
    _tools.mkdir("tools")
    outfile= "tools/imppy.sh"
    pythonpath=options.python_path.split(":")
    ldpath=options.ld_path.split(":")
    precommand=options.precommand
    modules=options.modules.split(":")
    path=options.path.split(os.pathsep)\
        +[os.path.abspath(os.path.join(options.base_dir,"module_bin",m)) for m in modules]\
        +[os.path.abspath(os.path.join(options.base_dir, "bin"))]
    externmodules=[x for x in options.external_modules.split(":") if x != ""]
    externdata=options.external_data
    if pythonpath == ['']: pythonpath = []
    if ldpath == ['']: ldpath = []
    if path == ['']: path = []

    libdir= os.path.abspath(os.path.join(options.base_dir, "lib"))
    impdir= os.path.join(libdir, "IMP")
    bindir= os.path.abspath(os.path.join(options.base_dir, "bin"))
    datadir= os.path.abspath(os.path.join(options.base_dir, "data"))
    exampledir=  os.path.abspath(os.path.join(options.base_dir, "doc", "examples"))
    tmpdir= os.path.abspath(os.path.join(options.base_dir, "tmp"))

    if platform.system() == 'Linux':
        varname= "LD_LIBRARY_PATH"
    elif platform.system() == 'Darwin':
        varname= "DYLD_LIBRARY_PATH"
    else:
        varname=None

    if 'kernel' not in modules:
        imp_module_path="IMP_MODULE_PATH"
    else:
        imp_module_path=None

    lines={"@LDPATH@":(varname, os.pathsep.join([libdir]+ldpath), True),
           "@PYTHONPATH@":("PYTHONPATH",
                           os.pathsep.join([libdir]+pythonpath), True),
           "@IMP_BIN_DIR@":("IMP_BIN_DIR", bindir, True),
           "@PATH@":("PATH", os.pathsep.join([bindir]+path), True),
           "@PRECOMMAND@":("precommand", precommand, False),
           "@MODULEPATH@":(imp_module_path, impdir, True),
           "@TMPDIR@":("IMP_TMP_DIR", tmpdir, True)}
    if options.wine_hack=="yes":
        lines['@LDPATH@'] = ('IMP_LD_PATH', os.pathsep.join(ldpath), True)

    contents=[]

    for line in template.split('\n'):
        line = line.rstrip('\r\n')

        if lines.has_key(line):
            val= lines[line]
            if val[0] and len(val[1])>0:
                contents.append(val[0]+"=\""+val[1]+"\"")
                if val[2]:
                    contents.append("export "+val[0])
        elif line== "@MODULEROOTS@":
            for m in modules:
                varname="IMP_"+m.upper()+"_DATA"
                contents.append(varname+"='"+datadir+"'")
                contents.append("export "+varname)
                varname="IMP_"+m.upper()+"_EXAMPLE_DATA"
                contents.append(varname+"='"+exampledir+"'")
                contents.append("export "+varname)
            for m in externmodules:
                varname="IMP_"+m.upper()+"_DATA"
                contents.append(varname+"='"+externdata+"'")
                contents.append("export "+ varname)
        else:
            contents.append(line)
    _tools.rewrite(outfile, "\n".join(contents))
    os.chmod(outfile, stat.S_IRWXU)

if __name__ == '__main__':
    main()
