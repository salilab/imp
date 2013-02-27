#!/usr/bin/env python

import IMP.em

def main():
    IMP.base.set_log_level(IMP.base.SILENT)
    usage = "%prog [options] <em map> "
    parser = IMP.OptionParser(usage=usage, imp_module=IMP.em)
    (options, args) = parser.parse_args()
    if len(args) != 1:
        parser.error("incorrect number of arguments")
    map_fn=args[0]
    dmap=IMP.em.read_map(map_fn)
    dmap.get_header().show()

if __name__ == "__main__":
    main()
