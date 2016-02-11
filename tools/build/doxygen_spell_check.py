#!/usr/bin/env python

"""Check spelling in doxygen-generated docs.

   This looks at the doxygen-generated XML documentation, extracts any
   human-readable text, and runs it through the enchant spellchecker.
"""

from __future__ import print_function
import sys
import os
import xml.etree.ElementTree as ET
import re
import glob
import array
import tools

try:
    import enchant.checker
    from enchant.tokenize import EmailFilter, URLFilter, Filter
except ImportError:
    print("Cannot check spelling - enchant library not available",
          file=sys.stderr)
    sys.exit(1)

class ClassNameFilter(Filter):
    """Exclude class names (CamelCase and/or containing ::)"""
    _pattern = re.compile(r"^([A-Z]\w+[A-Z]+\w+)")
    def _skip(self,word):
        if isinstance(word, array.array):
            word = word.tostring()
        return "::" in word or self._pattern.match(word) is not None


class SpellChecker(object):
    def __init__(self, exceptions_file):
        self.chkr = enchant.checker.SpellChecker("en_US",
                         filters=[EmailFilter, URLFilter,
                                  ClassNameFilter])
        self.add_exceptions(exceptions_file)
        self.fname = None
        self.xml_filename = None
        self.printed = False

    def add_exceptions(self, exceptions_file):
        d = {'spelling_exceptions': [], 'doc_spelling_exceptions': []}
        try:
            exec(open(exceptions_file, "r").read(), d)
        except IOError:
            pass
        for word in d['spelling_exceptions']:
            self.chkr.dict.add_to_session(word)
        for word in d['doc_spelling_exceptions']:
            self.chkr.dict.add_to_session(word)

    def check(self, text, node):
        """Check the given text from the given ElementTree node"""
        self.chkr.set_text(text)
        misspelled = set(err.word for err in self.chkr)
        if misspelled:
            if not self.printed:
                print("Misspelled words:", file=sys.stderr)
            self.printed = True
            fname, location = get_location(node)
            if fname is None:
                fname = self.xml_filename
            if fname != self.fname:
                print("  File " + fname, file=sys.stderr)
                self.fname = fname
            print("    %s:" % location, file=sys.stderr)
            print("       " + ", ".join(misspelled).encode('utf8'),
                  file=sys.stderr)

def extract_text(node):
    """Extract all human-readable text from a node and its children. This
       is like ElementTree.itertext() but works with Python 2.6 and excludes
       some text (such as program listings)"""
    text = []
    if node.tag not in ('programlisting', 'computeroutput', 'ref',
                        'parametername'):
        if node.text:
            text.append(node.text)
        for child in node:
            text.extend(extract_text(child))
    if node.tail:
        text.append(node.tail)
    return text

def get_description(node):
    """Get all human-readable description text for a node"""
    text = []
    for n in node.findall("description") + node.findall("briefdescription") \
             + node.findall("detaileddescription") \
             + node.findall("inbodydescription"):
        text.extend(extract_text(n))
    return " ".join(text)

def get_location(node):
    """Get a filename and location string for a node"""
    location = node.findall("location")
    definition = node.findall("definition")
    name = node.findall("compoundname")
    header = node.findall("header")
    # Use class name if available
    if name:
        text = name[0].text
    # Otherwise, use the method/function prototype
    elif definition:
        text = definition[0].text.replace('def ', '')
    elif header:
        text = 'Header "%s"' % header[0].text
    else:
        text = "Unknown entity"
    if location:
        fname = location[0].attrib['file']
        if os.path.islink(fname):
            fname = os.readlink(fname)
        if 'line' in location[0].attrib:
            text += " (line %s)" % location[0].attrib['line']
    else:
        fname = None
    return fname, text

def check_node(node, checker):
    description = get_description(node)
    if description:
        checker.check(description, node)

    for child in node:
        check_node(child, checker)

def main():
    if len(sys.argv) != 3:
        print("Usage: %s directory exceptions_file" % sys.argv[0],
              file=sys.stderr)
        sys.exit(1)

    checker = SpellChecker(sys.argv[2])
    for fname in glob.glob(os.path.join(sys.argv[1], "*.xml")):
        if fname.endswith('namespacestd.xml'):
            continue
        checker.xml_filename = fname
        with open(fname) as xml_file:
            tree = ET.parse(xml_file)
        root = tree.getroot()
        check_node(root, checker)

if __name__ == '__main__':
    main()
