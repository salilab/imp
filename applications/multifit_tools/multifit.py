#!/usr/bin/env python

import IMP

def main():
    c = IMP.CommandDispatcher(short_help='MultiFit.',
                              long_help='Fit complexes into a density map.',
                              module_name="IMP.multifit")
    c.main()

if __name__ == '__main__':
    main()
