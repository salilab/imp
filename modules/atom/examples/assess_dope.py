## \example atom/assess_dope.py
## The script shows how to assess a protein conformation using DOPE.

import IMP
import IMP.atom
import IMP.container

def create_representation():
    m=IMP.Model()
    mp0= IMP.atom.read_pdb(IMP.atom.get_example_path('1fdx.B99990001.pdb'), m, IMP.atom.NonWaterNonHydrogenPDBSelector())
    prot=IMP.atom.get_by_type(mp0, IMP.atom.CHAIN_TYPE)[0]
    return (m, prot)

def add_dope(m, prot):
    ps=IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)
    for p in ps:
        if not IMP.atom.Atom.particle_is_instance(p):
            print "Huh?", p
    dpc = IMP.container.ClosePairContainer(ps, 15.0, 0.0)
# exclude pairs of atoms belonging to the same residue
# for consistency with MODELLER DOPE score
    f=IMP.atom.SameResiduePairFilter()
    dpc.add_pair_filter(f)
    IMP.atom.add_dope_score_data(prot)
    dps= IMP.atom.DopePairScore(15.0)
#    dps= IMP.membrane.DopePairScore(15.0, IMP.membrane.get_data_path("dope_scorehr.lib"))
    d=   IMP.container.PairsRestraint(dps, dpc)
    m.add_restraint(d)

print "creating representation"
(m,prot)=create_representation()

print "creating DOPE score function"
add_dope(m,prot)

IMP.base.set_check_level(IMP.base.USAGE)
print "DOPE SCORE ::",m.evaluate(False)
