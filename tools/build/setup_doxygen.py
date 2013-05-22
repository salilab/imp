#!/usr/bin/env python

"""
Set up the things so doyxgen can be run:
- wrap the README.mds into .dox files
- generate the doxyfiles

No repository directories are changed.
"""

import os
import sys
import os.path
import shutil
import platform
import tools
from optparse import OptionParser

def doxygenize_readme(readme, output_dir, name):
    out = ["/**", "\\page IMP_%s_overview IMP.%s"%(name, name)]
    out.extend(open(readme, "r").read().split("\n"))
    out.append("*/")
    tools.rewrite(os.path.join(output_dir, "IMP_"+name+"_overview.dox"), "\n".join(out))

# link all the dox files and other documentation related files from the source tree
# into the build tree
def link_dox(source):
    target=os.path.join("doxygen")
    tools.mkdir(target)
    for module, g in tools.get_modules(source):
        tools.link_dir(os.path.join(g, "doc"), os.path.join(target, module))
        tools.link_dir(os.path.join(g, "doc"), os.path.join("doc", "html"), match=["*.png", "*.pdf"],
                 clean=False)
        doxygenize_readme(os.path.join(g, "README.md"), "doxygen", module)
    for app, g in tools.get_applications(source):
        tools.link_dir(g, os.path.join(target, app))
        tools.link_dir(g, os.path.join("doc", "html"), match=["*.png", "*.pdf"], exclude = ["README.md"], clean=False)
        doxygenize_readme(os.path.join(g, "README.md"), "doxygen", app)
    tools.link_dir(os.path.join(source, "doc"), os.path.join(target, "IMP"))
    tools.link_dir(os.path.join(source, "doc"), os.path.join("doc", "html"), match=["*.png", "*.pdf"],
             clean=False)


def generate_doxyfile(source):
    doxyin=os.path.join(source, "doc", "doxygen", "Doxyfile.in")
    version="develop"
    versionpath=os.path.join(source, "VERSION")
    if os.path.exists(versionpath):
        version= open(versionpath, "r").read().split('\n')[0].replace(" ", ":")
    # for building of modules without IMP
    if os.path.exists(doxyin):
        doxygen= open(doxyin, "r").read()
        doxygenr= doxygen.replace( "@IMP_SOURCE_PATH@", source).replace("@VERSION@", version)
        doxygenrhtml= doxygenr.replace( "@IS_HTML@", "YES").replace("@IS_XML@", "NO")
        doxygenrxml= doxygenr.replace( "@IS_XML@", "YES").replace("@IS_HTML@", "NO")
        open(os.path.join("doxygen", "Doxyfile.html"), "w").write(doxygenrhtml)
        open(os.path.join("doxygen", "Doxyfile.xml"), "w").write(doxygenrxml)

# generate the pages that list biological systems and applications
def generate_overview_pages(source):
    ai= open(os.path.join("doxygen", "applications.dox"), "w")
    ai.write("/** \\page applications_index Application Index \n")
    for bs, g in tools.get_applications(source):
        ai.write("- \\subpage IMP_%s_overview \"%s\"\n"%(bs,bs))
    ai.write("*/")
    ai= open(os.path.join("doxygen", "modules.dox"), "w")
    ai.write("/** \\page modules_index Module Index \n")
    for bs, g in tools.get_modules(source):
        ai.write("- \\subpage IMP_%s_overview \"%s\"\n"%(bs,bs))
    ai.write("*/")

def generate_changelog(source):
    input= open(os.path.join(source, 'ChangeLog.md'), "r").read()
    output= open(os.path.join("doxygen", 'ChangeLog.dox'), "w")
    output.write("/** \\page recent Change History\n")
    output.write(input)
    output.write("*/\n")

parser = OptionParser()
parser.add_option("-s", "--source", dest="source",
                  help="IMP source directory.")
def main():
    (options, args) = parser.parse_args()

    link_dox(options.source)
    generate_overview_pages(options.source)
    generate_doxyfile(options.source)
    generate_changelog(options.source)

if __name__ == '__main__':
    main()
