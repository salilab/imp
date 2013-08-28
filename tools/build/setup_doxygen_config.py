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

def generate_doxyfile(source, target, is_xml = False, is_html = False, tutorial=False):
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
        doxygen = doxygen.replace("@EXAMPLE_PATH@", ".")
        doxygen = doxygen.replace("@TAGS@", "doxygen/tags.html=../html")
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
        doxygen = doxygen.replace("@EXAMPLE_PATH@", "doc/examples %s/modules/example"%source)
        doxygen = doxygen.replace("@TAGS@", "")
    doxygen = doxygen.replace( "@NAME@", "IMP")
    doxygen = doxygen.replace( "@IMP_SOURCE_PATH@", source).replace("@VERSION@", version)
    doxygen = doxygen.replace("@EXCLUDE@", "")
    doxygen = doxygen.replace("@INCLUDE_PATH@", "include")
    doxygen = doxygen.replace("@XML_OUTPUT@", "doxygen/xml/")
    # TAGS, INPUT_PATH
    if is_xml:
        doxygen = doxygen.replace("@IS_XML@", "YES")
    else:
        doxygen = doxygen.replace("@IS_XML@", "NO")
    if is_html:
        doxygen = doxygen.replace( "@IS_HTML@", "YES")
    else:
        doxygen = doxygen.replace( "@IS_HTML@", "NO")

    # skip linking later
    inputsh = ["doxygen/generated", source + "/doc", source + "/ChangeLog.md",
               source + "/tools/README.md", "include", "doc/examples"]
    for m, p in tools.get_modules(source):
        doc = os.path.join(p, "doc")
        inputsh.append(os.path.join("lib", "IMP", m))
        if os.path.exists(doc):
            inputsh.append(doc + "/")
    for m, p in tools.get_applications(source):
        doc = os.path.join(p, "doc")
        if os.path.exists(doc):
            inputsh.append(doc + "/")
    if not tutorial:
        doxygen = doxygen.replace("@INPUT_PATH@", " ".join(inputsh))
    open(target, "w").write(doxygen)

# generate the pages that list biological systems and applications
def generate_overview_pages(source):
    name = os.path.join("doxygen", "generated", "all.dox")
    contents = []
    contents.append("/** ")
    contents.append("\\page All IMP Modules and Applications")
    contents.append("<table><tr>")
    contents.append("<th>Modules</th><th>Applications</th></tr><tr><td>")
    for bs, g in tools.get_modules(source):
        contents.append("- \\subpage imp%s \"IMP.%s\""%(bs,bs))
    contents.append("</td><td style=\"vertical-align:top;\">")
    for bs, g in tools.get_applications(source):
        contents.append("- \subpage imp%s \"IMP.%s\""%(bs,bs))
    contents.append("</td></tr></table>")
    contents.append("*/")
    tools.rewrite(name, "\n".join(contents))

parser = OptionParser()
parser.add_option("-s", "--source", dest="source",
                  help="IMP source directory.")
def main():
    (options, args) = parser.parse_args()

    generate_overview_pages(options.source)
    generate_doxyfile(options.source,
                      os.path.join("doxygen", "Doxyfile.html"),
                      is_html = True, is_xml = False)
    generate_doxyfile(options.source,
                      os.path.join("doxygen", "Doxyfile.xml"),
                      is_html = False, is_xml = True)
    generate_doxyfile(options.source,
                      os.path.join("doxygen", "tutorial.in"),
                      is_html = True, is_xml = False,
                      tutorial=True)

if __name__ == '__main__':
    main()
