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
import pickle
from optparse import OptionParser

class DoxConfigFileGenerator(tools.FileGenerator):
    def __init__(self, template_file):
        tools.FileGenerator.__init__(self, template_file, '#')

    def get_output_file_contents(self, output):
        is_xml = output['is_xml']
        is_html = output['is_html']
        manual = output['manual']
        source = output['source']
        version = "develop"
        versionpath = os.path.join("VERSION")
        if os.path.exists(versionpath):
            with open(versionpath, "r") as fh:
                version = fh.read().strip()
        if manual:
            version = '"for IMP version ' + version + '"'
        doxygen = self.template
        if manual:
            doxygen = doxygen.replace("@PROJECT_NAME@", '"IMP Manual"')
            doxygen = doxygen.replace("@PROJECT_BRIEF@", "")
            doxygen = doxygen.replace("@MAINPAGE@", "")
            doxygen = doxygen.replace("@RECURSIVE@", "YES")
            doxygen = doxygen.replace("@HTML_OUTPUT@", "doc/manual/")
            doxygen = doxygen.replace("@LAYOUT_FILE@",
                                  "%s/doc/doxygen/manual_layout.xml" % source)
            doxygen = doxygen.replace("@TREEVIEW@", "NO")
            doxygen = doxygen.replace("@GENERATE_TAGFILE@",
                                      "doxygen/manual-tags.xml")
            doxygen = doxygen.replace("@WARNINGS@",
                                      "doxygen/manual-warnings.txt")
            doxygen = doxygen.replace("@EXCLUDE_PATTERNS@", "")
            doxygen = doxygen.replace("@EXAMPLE_PATH@", ".")
            doxygen = doxygen.replace("@TAGS@", "doxygen/ref-tags.xml=../ref")
            doxygen = doxygen.replace("@XML_OUTPUT@", "doxygen/manual/xml/")
            doxygen = doxygen.replace("@EXCLUDE@", "")
        else:
            doxygen = doxygen.replace("@PROJECT_NAME@", '"IMP Reference Guide"')
            doxygen = doxygen.replace("@PROJECT_BRIEF@",
                                      '"The Integrative Modeling Platform"')
            doxygen = doxygen.replace("@MAINPAGE@", "mainpage.md")
            doxygen = doxygen.replace("@RECURSIVE@", "YES")
            doxygen = doxygen.replace("@HTML_OUTPUT@", "doc/ref/")
            doxygen = doxygen.replace("@LAYOUT_FILE@",
                                      "%s/doc/doxygen/main_layout.xml" % source)
            doxygen = doxygen.replace("@TREEVIEW@", "NO")
            doxygen = doxygen.replace("@GENERATE_TAGFILE@",
                                      "doxygen/ref-tags.xml")
            doxygen = doxygen.replace("@WARNINGS@", "doxygen/ref-warnings.txt")
            doxygen = doxygen.replace("@EXCLUDE_PATTERNS@", "")
            doxygen = doxygen.replace("@EXAMPLE_PATH@",
                                    "doc/examples %s/modules/example" % source)
            doxygen = doxygen.replace("@TAGS@",
                                      "doxygen/manual-tags.xml=../manual")
            doxygen = doxygen.replace("@XML_OUTPUT@", "doxygen/ref/xml/")
            doxygen = doxygen.replace("@EXCLUDE@",
                                      "lib/IMP/kernel lib/IMP/base")
        doxygen = doxygen.replace("@NAME@", "IMP")
        doxygen = doxygen.replace("@IMP_SOURCE_PATH@", source) \
                         .replace("@VERSION@", version)
        doxygen = doxygen.replace("@INCLUDE_PATH@", "include")
        doxygen = doxygen.replace("@FILE_PATTERNS@",
                                  "*.cpp *.h *.py *.md *.dox")
        # TAGS, INPUT_PATH
        if is_xml:
            doxygen = doxygen.replace("@IS_XML@", "YES")
        else:
            doxygen = doxygen.replace("@IS_XML@", "NO")
        if is_html:
            doxygen = doxygen.replace("@IS_HTML@", "YES")
        else:
            doxygen = doxygen.replace("@IS_HTML@", "NO")

        if manual:
            inputsh = [source + "/doc/manual",
                       source + "/tools/README.md",
                       source + "/ChangeLog.md"]
        else:
            inputsh = ["doxygen/generated", source + "/doc/ref",
                       "include", "doc/examples", "lib/IMP"]
            for m, p in tools.get_modules(source):
                doc = os.path.join(p, "doc")
                if os.path.exists(doc):
                    inputsh.append(doc + "/")
        doxygen = doxygen.replace("@INPUT_PATH@", " ".join(inputsh))
        return doxygen


def generate_doxyfile(source, target, is_xml=False, is_html=False,
                      manual=False):
    g = DoxConfigFileGenerator(os.path.join(source, "tools", "build",
                                            "doxygen_templates", "Doxyfile.in"))
    g.write(target, {'is_xml':is_xml, 'is_html':is_html, 'manual':manual,
                     'source':source},
            show_diff=False)

def generate_overview_pages(source):
    name = os.path.join("doxygen", "generated", "cmdline_tools.dox")
    contents = []
    contents.append("/** ")
    contents.append("\\page cmdline_tools All IMP command line tools")
    contents.append("""
IMP modules provide a number of command line tools.
These are listed below under their parent module:""")
    for bs, g in tools.get_modules(source):
        if tools.get_module_info(bs, '')['ok']:
            with open(os.path.join("build_info", "IMP_%s.pck" % bs),
                      'rb') as fh:
                p = pickle.load(fh)
            if len(p) > 0:
                contents.append("- IMP::%s" % bs)
            apps = sorted([[k]+list(v) for k,v in p.items() if v],
                          key=lambda x:x[3])
            for app in apps:
                contents.append("  - [%s](\\ref %s): %s" % (app[0], app[1],
                                                            app[2]))
    contents.append("""
See also the [command line tools provided by RMF](https://integrativemodeling.org/rmf/nightly/doc/executables.html).""")
    contents.append("*/")
    g = tools.DoxFileGenerator()
    g.write(name, "\n".join(contents))

parser = OptionParser()
parser.add_option("-s", "--source", dest="source",
                  help="IMP source directory.")


def main():
    (options, args) = parser.parse_args()

    generate_overview_pages(options.source)
    generate_doxyfile(options.source,
                      os.path.join("doxygen", "ref.html"),
                      is_html=True, is_xml=False)
    generate_doxyfile(options.source,
                      os.path.join("doxygen", "ref.xml"),
                      is_html=False, is_xml=True)
    generate_doxyfile(options.source,
                      os.path.join("doxygen", "manual.html"),
                      is_html=True, is_xml=False,
                      manual=True)
    generate_doxyfile(options.source,
                      os.path.join("doxygen", "manual.xml"),
                      is_html=False, is_xml=True,
                      manual=True)

if __name__ == '__main__':
    main()
