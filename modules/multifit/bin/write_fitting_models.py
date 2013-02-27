#!/usr/bin/env python
import sys,os,string
import IMP.multifit
import IMP.multifit2
from optparse import OptionParser
def usage():
    usage =  "usage %prog [options] <mol to fit> <transformations> <start trans (default is first)> <end trans (default is last)>\n"
    usage+="A script for getting fits with best RMSD to the reference"
    parser = OptionParser(usage)
    parser.add_option("-d", action="store_true", dest="use_dock",
                      help="if set the docking transformation is used (and not the fitting transformation)")
    parser.add_option("-m", dest="dir", default="./",
                      help="output models directory")
    (options, args) = parser.parse_args()
    if len(args) < 2:
        parser.error("incorrect number of arguments")
    return [options,args]

def run(mol_fn,trans_fn,first,last,dock_trans,out_dir):
    IMP.base.set_log_level(IMP.base.SILENT)
    mdl=IMP.Model()
    print mol_fn
    mh=IMP.atom.read_pdb(mol_fn,mdl)
    xyz=IMP.core.XYZs(IMP.core.get_leaves(mh))
    IMP.atom.create_rigid_body(mh)
    rb=IMP.core.RigidMember(xyz[0]).get_rigid_body()
    name=mol_fn.split("/")[-1].split(".pdb")[0]
    print name, trans_fn
    fits=IMP.multifit.read_fitting_solutions(trans_fn)
    print "=======2"
    for i,rec in enumerate(fits[first:last]):
        fit_t=rec.get_fit_transformation()
        if dock_trans:
            fit_t=rec.get_dock_transformation()
        IMP.core.transform(rb,fit_t)
        IMP.atom.write_pdb(mh,out_dir+name+"."+str(i)+".pdb")
        IMP.core.transform(rb,fit_t.get_inverse())
    print "=======3"
if __name__=="__main__":
    (options,args) = usage()
    first=0
    last=99999999
    if len(args)==3 or len(args)==4:
        args[2]
        first=int(args[2])
    if len(args)==4:
        last=int(args[3])
    run(args[0],args[1],first,last,options.use_dock,options.dir)
