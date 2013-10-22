#!/usr/bin/python

"""Create doxygen config files for the tutorial"""

import xml.dom.minidom
import os
import sys
import re
import glob

def parse_mainpage(fh):
    """Find all links to local pages in the tutorial main page, and
       return the doxygen page names"""
    pages = []
    r = re.compile('\[.*\]\(([^/]*)\.html\)')
    for line in fh:
        m = r.search(line)
        if m:
            pages.append(m.group(1))
    return pages

def find_pages_in_file(fname, pages, page_map):
    """Look through the given doxygen file, and see if it matches any of
       the given doxygen page names"""
    r = re.compile('\\page (\w+)')
    for line in open(fname):
        m = r.search(line)
        if m:
            if m.group(1) in pages:
                page_map[m.group(1)] = fname
            return

def find_pages(topdir, pages):
    """Get the mapping from doxygen page names to actual files on disk"""
    page_map = dict.fromkeys(pages)
    for dirpath, dirnames, filenames in os.walk(topdir):
        for f in filenames:
            if f.endswith('.dox'):
                fullname = os.path.join(dirpath, f)
                find_pages_in_file(fullname, pages, page_map)
    for key, val in page_map.items():
        if val is None:
            raise RuntimeError("Cannot find page %s" % key)
    return page_map

def make_doxyfile(infh, outfh, mainpage, pages, page_map):
    """Generate the doxygen config file from the template"""
    inputs = " \\\n".join([mainpage] + [page_map[p] for p in pages])
    for line in infh:
        line = line.replace('@INPUT_PATH@', inputs)
        outfh.write(line)

ourdir = os.path.abspath(os.path.dirname(sys.argv[0]))
mainpage = os.path.join(ourdir, 'mainpage.dox')
pages = parse_mainpage(open(mainpage))
page_map = find_pages(ourdir, pages)
make_doxyfile(open('doxygen/tutorial.in'),
              open('doxygen/tutorial', 'w'), mainpage, pages, page_map)
