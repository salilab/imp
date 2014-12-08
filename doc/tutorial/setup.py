#!/usr/bin/env python

"""Create doxygen config files for the tutorial. List files explictly so
   that the left pane of the generated HTML looks tidy."""

import xml.dom.minidom
import os
import sys
import re
import glob


def parse_mainpage(fh):
    """Find all links to local pages in the tutorial main page, and
       return the doxygen page names"""
    pages = []
    r = re.compile('@ref\s+(\w+)')
    for line in fh:
        m = r.search(line)
        if m:
            pages.append(m.group(1))
    return pages

def get_doxygen_name(fname):
    """Get the doxygen page name from a file"""
    if fname.endswith('.dox'):
        r = re.compile('\\page (\w+)')
    elif fname.endswith('.md'):
        r = re.compile('{#([^}]+)}')
    else:
        return None
    for line in open(fname):
        m = r.search(line)
        if m:
            return m.group(1)

def find_pages(topdir, pages, mainpage):
    """Get the mapping from doxygen page names to actual files on disk"""
    page_map = dict.fromkeys(pages)
    for dirpath, dirnames, filenames in os.walk(topdir):
        for f in filenames:
            fullname = os.path.join(dirpath, f)
            if fullname == mainpage:
                continue
            dox_name = get_doxygen_name(fullname)
            if dox_name:
                if dox_name not in page_map:
                    pages.append(dox_name)
                page_map[dox_name] = fullname
    for key, val in page_map.items():
        if val is None:
            raise RuntimeError("Cannot find page %s" % key)
    return pages, page_map


def make_doxyfile(infh, outfh, mainpage, pages, page_map):
    """Generate the doxygen config file from the template"""
    inputs = " \\\n".join([mainpage] + [page_map[p] for p in pages])
    for line in infh:
        line = line.replace('@INPUT_PATH@', inputs)
        outfh.write(line)

ourdir = os.path.abspath(os.path.dirname(sys.argv[0]))
mainpage = os.path.join(ourdir, 'mainpage.dox')
pages = parse_mainpage(open(mainpage))
pages, page_map = find_pages(ourdir, pages, mainpage)
make_doxyfile(open('doxygen/tutorial.in'),
              open('doxygen/tutorial', 'w'), mainpage, pages, page_map)
