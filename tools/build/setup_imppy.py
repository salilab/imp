#!/usr/bin/python
import _tools
from optparse import OptionParser
import os.path
import os
import platform
import stat
import glob

template = """#!/bin/sh

@LDPATH@

@PYTHONPATH@

# Where to find data for the various modules
@IMP_DATA@

# Extra places to look for imp modules
@IMP_EXAMPLE_DATA@

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
parser.add_option("-P", "--path", dest="path", default="",
                  help="The PATH.")
parser.add_option("-d", "--external_data", dest="external_data", default="",
                  help="External data.")
parser.add_option("-W", "--wine_hack", dest="wine_hack", default="no",
                  help="Base dir, either . or build.")

def main():
    (options, args) = parser.parse_args()
    _tools.mkdir("tools")
    outfile= "imppy.sh"
    pythonpath=_tools.split(options.python_path)
    ldpath=_tools.split(options.ld_path)
    precommand=options.precommand
    path= [os.path.abspath(x) for x in glob.glob("module_bin/*")]\
        + [os.path.abspath("bin")] + _tools.split(options.path)
    externdata=_tools.split(options.external_data)

    libdir= os.path.abspath("lib")
    impdir= os.path.join(libdir, "IMP")
    bindir= os.path.abspath("bin")
    datadir= os.path.abspath("data")
    exampledir=  os.path.abspath(os.path.join("doc", "examples"))
    tmpdir= os.path.abspath("tmp")

    if platform.system() == 'Linux':
        varname= "LD_LIBRARY_PATH"
    elif platform.system() == 'Darwin':
        varname= "DYLD_LIBRARY_PATH"
    else:
        varname=None

    lines={"@LDPATH@":(varname, os.pathsep.join([libdir]+ldpath), True),
           "@PYTHONPATH@":("PYTHONPATH",
                           os.pathsep.join([libdir]+pythonpath), True),
           "@IMP_BIN_DIR@":("IMP_BIN_DIR", bindir, True),
           "@PATH@":("PATH", os.pathsep.join([bindir]+path), True),
           "@PRECOMMAND@":("precommand", precommand, False),
           "@IMP_DATA@":("IMP_DATA", os.pathsep.join([datadir] + externdata), True),
           "@IMP_EXAMPLE_DATA@":("IMP_EXAMPLE_DATA", os.pathsep.join([exampledir]), True),
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
        else:
            contents.append(line)
    _tools.rewrite(outfile, "\n".join(contents))
    os.chmod(outfile, stat.S_IRWXU)

if __name__ == '__main__':
    main()
