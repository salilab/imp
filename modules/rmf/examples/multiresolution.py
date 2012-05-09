import IMP.atom
import RMF
import IMP.rmf
pdbname= IMP.rmf.get_example_path("big.pdb")

m= IMP.Model()
h= IMP.atom.read_pdb(pdbname, m)

chains= IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)

def recursive_approximation(res):
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

fn= IMP.base.create_temporary_file_name("multires", ".rmf")
rmf= RMF.create_rmf_file(fn)
IMP.rmf.add_hierarchies(rmf, chains)

print "see file", fn
