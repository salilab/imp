#!/usr/bin/env python

__doc__ = "Generate proteomics info from anchor graph and fits."

#read the anchors
#read the top fit for each protein, and assign the anchors
#add EV accordinly

import IMP.multifit
from optparse import OptionParser

def parse_args():
    usage = """%prog [options] <asmb.input> <anchors.txt>
                                             <output:proteomics>

Generate a proteomics file automatically from the anchor graph and fitting
results. No interaction data is entered here, but the file can be modified
manually afterwards to add additional proteomics information.
"""
    parser = OptionParser(usage)
    options, args = parser.parse_args()
    if len(args) != 3:
        parser.error("incorrect number of arguments")
    return args

def run(asmb_fn,anchors_fn,proteomics_fn):
    asmb=IMP.multifit.read_settings(asmb_fn)
    asmb.set_was_used(True)
    ad=IMP.multifit.read_anchors_data(anchors_fn)

    #read molecules
    mdl=IMP.Model()
    mhs=[]
    centroids=[]
    for i in range(asmb.get_number_of_component_headers()):
        fn = asmb.get_component_header(i).get_filename()
        mhs.append(IMP.atom.read_pdb(fn, mdl))
        centroids.append(IMP.core.get_centroid(IMP.core.get_leaves(mhs[i])))
    #matched anchors
    match=[]
    for pt in ad.points_:
        min_len=999999
        min_ind=0
        for j in range(len(mhs)):
            dist = IMP.algebra.get_squared_distance(pt,centroids[j])
            if dist < min_len:
                min_len = dist
                min_ind = j
        match.append(min_ind)
    #now add all the EV
    ev_pairs=[]
    for ind1,ind2 in ad.edges_:
        ev_pairs.append([match[ind1],match[ind2]])
    outf=open(proteomics_fn,"w")
    outf.write("|proteins|\n")
    for i,mh in enumerate(mhs):
        numres = len(IMP.atom.get_by_type(mh,IMP.atom.RESIDUE_TYPE))
        outf.write("|%s|1|%d|nn|nn|\n" \
                   % (asmb.get_component_header(i).get_name(), numres))
    outf.write("|interactions|\n")
    outf.write("|residue-xlink|\n")
    outf.write("|ev-pairs|\n")
    pairs_map={}
    for evp in ev_pairs:
        if evp[0] != evp[1]:
            sortpair = (min(*evp), max(*evp))
            if sortpair not in pairs_map:
                name0 = asmb.get_component_header(evp[0]).get_name()
                name1 = asmb.get_component_header(evp[1]).get_name()
                outf.write("|%s|%s|\n" % (name0, name1))
                pairs_map[sortpair]=1
    outf.close()

def main():
    asmb_fn, anchors_fn, proteomics_fn = parse_args()
    run(asmb_fn, anchors_fn, proteomics_fn)

if __name__=="__main__":
    main()
