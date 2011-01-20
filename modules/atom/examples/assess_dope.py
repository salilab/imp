import IMP
import IMP.atom
import IMP.container
import IMP.membrane

def create_representation():
    m=IMP.Model()
    mp0= IMP.atom.read_pdb('1fdx.B99990001.pdb', m, IMP.atom.NonWaterNonHydrogenPDBSelector())
    prot=IMP.atom.get_by_type(mp0, IMP.atom.CHAIN_TYPE)[0]
    return (m, prot)

def add_dope(m, prot):
    ps=IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)
    dsc= IMP.container.ListSingletonContainer(m)
    dsc.add_particles(ps)
    dpc = IMP.container.ClosePairContainer(dsc, 15.0, 0.0)
# exclude pairs of atoms belonging to the same residue
# for consistency with MODELLER DOPE score
    f=IMP.membrane.SameResiduePairFilter()
    dpc.add_pair_filter(f)
    IMP.membrane.add_dope_score_data(prot)
    dps= IMP.membrane.DopePairScore(15.0)
#    dps= IMP.membrane.DopePairScore(15.0, IMP.membrane.get_data_path("dope_scorehr.lib"))
    d=   IMP.container.PairsRestraint(dps, dpc)
    m.add_restraint(d)

print "creating representation"
(m,prot)=create_representation()

print "creating DOPE score function"
add_dope(m,prot)

print "DOPE SCORE ::",m.evaluate(False)
