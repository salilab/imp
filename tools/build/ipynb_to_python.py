#!/usr/bin/env python

"""Convert an ipython notebook (.ipynb) file to plain Python (.py)"""

import json
import sys

def dump_cells(cells, sourcekey, of):
    for i, cell in enumerate(cells):
        if cell['cell_type'] == 'code':
            for line in cell[sourcekey]:
                of.write(line)
            of.write('\n\n')
        elif cell['cell_type'] == 'heading':
            for line in cell['source']:
                of.write('# ' + line)
            of.write('\n\n')

j = json.load(sys.stdin)
of = sys.stdout
if j['nbformat'] >= 4:
    dump_cells(j['cells'], 'source', of)
else:
    dump_cells(j['worksheets'][0]['cells'], 'input', of)
