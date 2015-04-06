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

def get_all_listitems(top_element, parent=None):
    for child in top_element:
        if child.tag == 'listitem':
            yield child, parent
            for x in get_all_listitems(child, child):
                yield x
        else:
            for x in get_all_listitems(child, parent):
                yield x

def get_all_links(item):
    for para in item.findall("para"):
        for ref in para.findall("ref"):
            if ref.attrib['kindref'] == 'compound' \
               and not 'external' in ref.attrib:
                yield ref

def get_contents(root, page_map):
    parent_pages = {}
    for item, parent in get_all_listitems(root):
        if parent is not None and parent not in parent_pages:
            links = list(get_all_links(parent))
            if len(links) > 0:
                parent_pages[parent] = page_map[links[0].attrib['refid']]
            else:
                parent_pages[parent] = None
        for link in get_all_links(item):
            yield page_map[link.attrib['refid']], link.text, parent_pages.get(parent, None)

def write_children(children, out):
    out.write('<ul>\n')
    for c in children:
        out.write('<li><a class="el" href="%s.html">%s</a></li>\n'
                  % (c[0], c[1]))
    out.write('</ul>\n')

def add_page_navigation(html_dir, pagename, children, prevpage, nextpage,
                        uppage):
    def make_link(title, img, dest):
        return '<a href="%s.html" title="%s"><img src="%s.png" alt="%s"></a>' \
               % (dest, title, img, img)
    links = []
    if prevpage:
        links.append(make_link("Go to previous page", "prev", prevpage))
    if uppage:
        links.append(make_link("Go to parent page", "up", uppage))
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
        elif '[SUBPAGES]' in line:
            write_children(children, out)
        else:
            out.write(line)

def get_page_children(i, contents):
    children = []
    for ind in range(i + 1, len(contents)):
        if contents[ind][2] == contents[i][0]:
            children.append(contents[ind][:2])
        else:
            break
    return children

def main():
    xml_dir = 'doxygen/manual/xml'
    html_dir = 'doc/manual'
    page_map = map_ids_to_pages(xml_dir)
    tree = ET.parse(os.path.join(xml_dir, 'indexpage.xml'))
    root = tree.getroot()

    contents = list(get_contents(root, page_map))
    for i, page in enumerate(contents):
        children = get_page_children(i, contents)
        add_page_navigation(html_dir, page[0], children,
                            contents[i-1][0] if i > 0 else 'index',
                            contents[i+1][0] if i+1 < len(contents) else None,
                            contents[i][2] if contents[i][2] else 'index')
    if len(contents) > 0:
        add_page_navigation(html_dir, 'index', [], None, contents[0][0], None)

if __name__ == '__main__':
    main()
