#!/usr/bin/env python

"""Add navigation to the reference guide HTML output, similarly
   to what is done for the manual (see doxygen_add_nav.py).
"""

from doxygen_add_nav import Docs, Page
import tools
import sys
import os


def all_module_docs(modules):
    return [os.path.join('modules', m, 'doc') for m in modules]


def get_module_readme(module):
    """Make a Page object for a module's top-level README file"""
    modulesub = module.replace('_', '__')
    p = Page('namespace' + modulesub)
    if module == 'kernel':
        p.out_file_name = 'namespaceIMP'
    else:
        p.out_file_name = 'namespaceIMP_1_1' + modulesub
    p.source_file_name = 'modules/%s/README.md' % module
    return p


def main():
    source = sys.argv[1]
    # Exclude obsolete modules
    modules = [m[0] for m in tools.get_modules(source) if m[0] != 'pmi1']
    docs = Docs(xml_dir='doxygen/ref/xml',
                html_dir='doc/ref',
                top_source_dir=source,
                source_subdirs=sys.argv[2:] + all_module_docs(modules))
    for p in docs.pages:
        p.map_location_to_source(source)
    for m in modules:
        docs.pages.append(get_module_readme(m))

    # Difficult to figure out the main/index page, so set this manually
    p = docs.page_by_id['indexpage']
    p.out_file_name = 'index'
    p.source_file_name = 'doc/ref/mainpage.md'

    for p in docs.pages:
        if docs.get_html_pages(p):
            docs.add_page_navigation(p)
        else:
            print("Not handling %s: HTML file does not exist" % p.id)


if __name__ == '__main__':
    main()
