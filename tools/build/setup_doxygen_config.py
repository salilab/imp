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

def generate_doxyfile(source, tutorial=False):
    doxyin=os.path.join(source, "tools", "build", "doxygen_templates", "Doxyfile.in")
    version="develop"
    versionpath=os.path.join(source, "VERSION")
    if os.path.exists(versionpath):
        version= open(versionpath, "r").read().split('\n')[0].replace(" ", ":")
    if tutorial:
        version = '"for IMP version ' + version + '"'
    doxygen = open(doxyin, "r").read()
    if tutorial:
        doxygen = doxygen.replace("@PROJECT_NAME@", '"IMP Tutorial"')
        doxygen = doxygen.replace("@PROJECT_BRIEF@", "")
        doxygen = doxygen.replace("@MAINPAGE@", "")
        doxygen = doxygen.replace("@RECURSIVE@", "NO")
        doxygen = doxygen.replace("@HTML_OUTPUT@", "doc/tutorial/")
        doxygen = doxygen.replace("@LAYOUT_FILE@", "")
        doxygen = doxygen.replace("@GENERATE_TAGFILE@", "")
        doxygen = doxygen.replace("@WARNINGS@", "doxygen/tutorial-warnings.txt")
        doxygen = doxygen.replace("@EXCLUDE_PATTERNS@", "")
        out_fname = os.path.join("doxygen", "tutorial.in")
        def make_tag(subdir):
            return os.path.join("doxygen", subdir, "tags-tutorial") \
                   + "=../html/" + subdir
    else:
        doxygen = doxygen.replace("@PROJECT_NAME@", "IMP")
        doxygen = doxygen.replace("@PROJECT_BRIEF@",
                                  '"The Integrative Modeling Platform"')
        doxygen = doxygen.replace("@MAINPAGE@", "mainpage.md")
        doxygen = doxygen.replace("@RECURSIVE@", "YES")
        doxygen = doxygen.replace("@HTML_OUTPUT@", "doc/html/")
        doxygen = doxygen.replace("@LAYOUT_FILE@",
                                  "%s/doc/doxygen/main_layout.xml" % source)
        doxygen = doxygen.replace("@GENERATE_TAGFILE@", "doxygen/tags.html")
        doxygen = doxygen.replace("@WARNINGS@", "doxygen/warnings.txt")
        doxygen = doxygen.replace("@EXCLUDE_PATTERNS@", "*/tutorial/*")
        out_fname = os.path.join("doxygen", "Doxyfile.html")
        def make_tag(subdir):
            return os.path.join("doxygen", subdir, "tags") + "=" + subdir
    doxygen = doxygen.replace( "@NAME@", "IMP")
    doxygen = doxygen.replace( "@IMP_SOURCE_PATH@", source).replace("@VERSION@", version)
    doxygen = doxygen.replace("@EXCLUDE@", "")
    doxygen = doxygen.replace("@INCLUDE_PATH@", "include")
    doxygen = doxygen.replace("@XML_OUTPUT@", "doc/xml/")
    # EXAMPLE_PATH, TAGS, INPUT_PATH
    doxygen = doxygen.replace( "@IS_HTML@", "YES").replace("@IS_XML@", "NO")

    doxygen = doxygen.replace("@EXAMPLE_PATH@", "")
    tags = []
    for m, g in tools.get_modules(source):
        if tools.get_module_info(m, "")["ok"]:
            tags.append(make_tag(m))
    for a, g in tools.get_applications(source):
        if tools.get_application_info(a, "")["ok"]:
            tags.append(make_tag(a))
    doxygen = doxygen.replace("@TAGS@", " ".join(tags))
    # skip linking later
    inputsh = ["doxygen", source + "/doc", source + "/ChangeLog.md",
               source + "/tools/README.md"]
    if not tutorial:
        doxygen = doxygen.replace("@INPUT_PATH@", " ".join(inputsh))
    doxygen = doxygen.replace("@FILE_PATTERNS@", "*.md *.dox")
    open(out_fname, "w").write(doxygen)

# generate the pages that list biological systems and applications
def generate_overview_pages(source):
    ai= open(os.path.join("doxygen", "all.md"), "w")
    ai.write("# All IMP Modules and Applications # {#all}\n")
    ai.write("<table><tr>\n")
    ai.write("<th>Modules</th><th>Applications</th></tr><tr><td>\n")
    for bs, g in tools.get_modules(source):
        ai.write("- [IMP.%s](%s/index.html)\n"%(bs,bs))
    ai.write("</td><td style=\"vertical-align:top;\">\n")
    for bs, g in tools.get_applications(source):
        ai.write("- [IMP.%s](%s/index.html)\n"%(bs,bs))
    ai.write("</td></tr></table>\n")

parser = OptionParser()
parser.add_option("-s", "--source", dest="source",
                  help="IMP source directory.")
def main():
    (options, args) = parser.parse_args()

    generate_overview_pages(options.source)
    generate_doxyfile(options.source)
    generate_doxyfile(options.source, tutorial=True)

if __name__ == '__main__':
    main()
