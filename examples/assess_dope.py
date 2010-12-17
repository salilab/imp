import IMP
import IMP.atom
import IMP.membrane

def create_representation():
    m=IMP.Model()
    mp0= IMP.atom.read_pdb('1fdx.B99990001.pdb', m, IMP.atom.NonWaterNonHydrogenPDBSelector())
    chain=IMP.atom.get_by_type(mp0, IMP.atom.CHAIN_TYPE)[0]
    return (m, chain)

def add_DOPE(m, chain):
    dsc= IMP.container.ListSingletonContainer(m)
    dsc.add_particles(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE))
    IMP.membrane.add_dope_score_data(chain)
    dpc= IMP.container.ClosePairContainer(dsc, 30.0, 0.0)
    dps= IMP.membrane.DopePairScore(30.0)
    d=   IMP.container.PairsRestraint(dps, dpc)
    m.add_restraint(d)

# Here starts the real job...
print "creating representation"
(m,chain)=create_representation()

print "creating DOPE score function"
add_DOPE(m,chain)

print "update model"
m.update()
