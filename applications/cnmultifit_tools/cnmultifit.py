#!/usr/bin/env python

import IMP

def main():
    c = IMP.CommandDispatcher(short_help='Symmetric MultiFit.',
                              long_help='Fit symmetric complexes '
                                        'into a density map.',
                              module_name="IMP.cnmultifit")
    c.main()

if __name__ == '__main__':
    main()
