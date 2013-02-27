#!/usr/bin/env python

from optparse import OptionParser
import IMP.em
import IMP.multifit
def main():
    IMP.base.set_log_level(IMP.base.SILENT)
    usage = "usage: %prog [options] <complex.pdb> <output: anchor_graph.cmm>\n Description: The script gets as input a PDB file of a complex and calculates an anchor graph,\n such that nodes corresponds to the centroids of its chains and \n edges are between chains that are close in space."
    parser = OptionParser(usage)
    (options, args) = parser.parse_args()
    if len(args) != 2:
        parser.error("incorrect number of arguments")
    pdb_fn=args[0]
    cmm_fn=args[1]
    #read the protein
    mdl=IMP.Model()
    mh=IMP.atom.read_pdb(pdb_fn,mdl,IMP.atom.CAlphaPDBSelector())
    IMP.atom.add_radii(mh)
    #find the centers of the individual chains
    chains=IMP.atom.get_by_type(mh,IMP.atom.CHAIN_TYPE)
    centers=[]
    rbs=[]
    for chain in chains:
        ps=IMP.core.XYZs(IMP.core.get_leaves(chain))
        centers.append(IMP.core.get_centroid(ps))
        IMP.atom.setup_as_rigid_body(chain)
    #fast calculations of which chains are close
    links=[]
    sdps= IMP.core.SphereDistancePairScore(IMP.core.Linear(0,1))
    rdps= IMP.core.RigidBodyDistancePairScore(sdps, IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))
    for id1,chain1 in enumerate(chains):
        for id2,chain2 in enumerate(chains[id1+1:]):
            #check if the bounding boxes are intersecting
            if rdps.evaluate((chain1.get_particle(), chain2), None)<0.5:
                links.append([id1,id1+id2+1])
    #write the cmm file
    output=open(cmm_fn,"w")
    output.write('<marker_set name="centers_cryst">\n')
    for id,c in enumerate(centers):
        output.write('<marker id="'+str(id)+'" x="'+str(c[0])+'" y="'+str(c[1])+'" z="'+str(c[2])+ '" radius= "3.0" r= "0.9"  g= "0.05" b= "0.18" />')
    for id1,id2 in links:
        output.write('<link id1="'+str(id1)+'" id2="'+str(id2)+'" radius="1."/>\n')
    output.write('</marker_set>\n')

if __name__ == "__main__":
    main()
