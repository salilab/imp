## \example rmf/multiresolution.py
## In this example a pdb is converted into a multiresolution model, somewhat painfully.

import IMP.atom
import IMP.base
import sys
import RMF
import IMP.rmf

IMP.base.setup_from_argv(sys.argv, "Create a multiresolution rmf file")

pdbname= IMP.rmf.get_example_path("big.pdb")

m= IMP.Model()
h= IMP.atom.read_pdb(pdbname, m)
IMP.atom.add_bonds(h)

chains= IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)

if IMP.base.get_bool_flag("run_quick_test"):
    chains=[chains[0]]

def recursive_approximation(res):
    print "approximating", res
    lr=len(res)
    if lr<=1:
        return res
    if lr > 4:
        me= recursive_approximation(res[0:lr/4])\
            +recursive_approximation(res[lr/4: lr/2])\
            +recursive_approximation(res[lr/2: 3*lr/4])\
            +recursive_approximation(res[3*lr/4: lr])
    else:
        me= res
    p= IMP.Particle(m)
    hc=IMP.atom.Hierarchy.setup_particle(p)
    IMP.atom.setup_as_approximation(p, res)
    nm= str(IMP.atom.Residue(res[0]).get_index())+"-"\
        + str(IMP.atom.Residue(res[-1]).get_index())
    p.set_name(nm)
    for mm in me:
        hc.add_child(mm)
    return [hc]

for c in chains:
    res= IMP.atom.get_by_type(h, IMP.atom.RESIDUE_TYPE)
    for r in res:
        c.remove_child(r)
        lvs= IMP.atom.get_leaves(r)
        IMP.atom.setup_as_approximation(r, lvs)
    lr=len(res)
    me= recursive_approximation(res[0:lr/4])\
        +recursive_approximation(res[lr/4: lr/2])\
        +recursive_approximation(res[lr/2: 3*lr/4])\
        +recursive_approximation(res[3*lr/4: lr])
    for mm in me:
        c.add_child(mm)

print "writing"
fn= IMP.base.create_temporary_file_name("multires", ".rmf")
rmf= RMF.create_rmf_file(fn)
print "adding"
IMP.rmf.add_hierarchies(rmf, chains)

print "see file", fn
