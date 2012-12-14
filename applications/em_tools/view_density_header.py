#!/usr/bin/python

from optparse import OptionParser
import IMP.em
def main():
    IMP.set_log_level(IMP.SILENT)
    usage = "usage: %prog [options] <em map> "
    parser = OptionParser(usage)
    (options, args) = parser.parse_args()
    if len(args) != 1:
        parser.error("incorrect number of arguments")
    map_fn=args[0]
    dmap=IMP.em.read_map(map_fn)
    dmap.get_header().show()

if __name__ == "__main__":
    main()
