#!/usr/bin/env python

import IMP.em
import IMP.atom

def main():
    usage = """%prog [options] <em map>
            <number of residues>

Approximate the density threshold to use given the molecular mass of
the complex."""
    parser = IMP.OptionParser(usage=usage, imp_module=IMP.em)
    parser.add_option("-p", "--apix", dest="apix",
                      help="voxel size")
    (options, args) = parser.parse_args()
    if len(args) != 2:
        parser.error("incorrect number of arguments")
    in_map_fn=args[0]
    num_res=int(args[1])
    #read the map
    dmap=IMP.em.read_map(in_map_fn)
    if options.apix:
        dmap.update_voxel_size(float(options.apix))

    mass=IMP.atom.get_mass_from_number_of_residues(num_res)

    t=IMP.em.get_threshold_for_approximate_mass(dmap,mass)
    print "Mass",mass
    print "approximated threshold:",t

if __name__=="__main__":
    main()
