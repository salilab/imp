#!/usr/bin/env python

"""Parse the manual's XML output to get the contents. Then patch each page
   in the HTML output to add navigation links (next page, previous page)
"""

import xml.etree.ElementTree as ET
import glob
import os

def map_ids_to_pages(xml_dir):
    mapping = {}
    for f in glob.glob(os.path.join(xml_dir, "*.xml")):
        tree = ET.parse(f)
        root = tree.getroot()
        compounddef = root.find('compounddef')
        if compounddef is not None and compounddef.attrib['kind'] == 'page':
            # Remove .xml suffix
            mapping[compounddef.attrib['id']] = os.path.basename(f)[:-4]
    return mapping

def get_all_listitems(top_element, level=1):
    for child in top_element:
        if child.tag == 'listitem':
            yield level, child
            for x in get_all_listitems(child, level + 1):
                yield x
        else:
            for x in get_all_listitems(child, level):
                yield x

def get_contents(root, page_map):
    for level, item in get_all_listitems(root):
        for para in item.findall("para"):
            for ref in para.findall("ref"):
                if ref.attrib['kindref'] == 'compound' \
                   and not 'external' in ref.attrib:
                    yield level, page_map[ref.attrib['refid']]

def add_page_navigation(html_dir, pagename, prevpage, nextpage):
    def make_link(title, img, dest):
        return '<a href="%s.html" title="%s"><img src="%s.png" alt="%s"></a>' \
               % (dest, title, img, img)
    links = []
    if prevpage:
        links.append(make_link("Go to previous page", "prev", prevpage))
    if nextpage:
        links.append(make_link("Go to next page", "next", nextpage))
    links = '<div class="doxnavlinks">' + " ".join(links) + '</div>\n'
    fname = os.path.join(html_dir, pagename + '.html')
    content = open(fname).readlines()
    out = open(fname, 'w')
    for line in content:
        if line.startswith('</div><!-- top -->'):
            out.write(line)
            out.write(links)
        elif line.startswith('<hr class="footer"'):
            out.write('<hr class="footer"/>')
            out.write(links)
        else:
            out.write(line)

def main():
    xml_dir = 'doxygen/manual/xml'
    html_dir = 'doc/manual'
    page_map = map_ids_to_pages(xml_dir)
    tree = ET.parse(os.path.join(xml_dir, 'indexpage.xml'))
    root = tree.getroot()

    contents = list(get_contents(root, page_map))
    for i, page in enumerate(contents):
        add_page_navigation(html_dir, page[1],
                            contents[i-1][1] if i > 0 else 'index',
                            contents[i+1][1] if i+1 < len(contents) else None)
    if len(contents) > 0:
        add_page_navigation(html_dir, 'index', None, contents[0][1])

if __name__ == '__main__':
    main()
