import IMP
import IMP.atom
import IMP.container
import IMP.membrane

def create_representation():
    m=IMP.Model()
    mp0= IMP.atom.read_pdb('1fdx.B99990001.pdb', m, IMP.atom.NonWaterNonHydrogenPDBSelector())
#    prot= IMP.atom.read_pdb('simple.pdb', m, IMP.atom.NonWaterNonHydrogenPDBSelector())
    prot=IMP.atom.get_by_type(mp0, IMP.atom.CHAIN_TYPE)[0]
    return (m, prot)

def add_DOPE(m, prot):
    ff = IMP.atom.get_heavy_atom_CHARMM_parameters()
    topology = ff.create_topology(prot)
    topology.apply_default_patches(ff)
    topology.add_atom_types(prot)
    bonds = topology.add_bonds(prot, ff)
    angles = ff.create_angles(bonds)
    dihedrals = ff.create_dihedrals(bonds)

    dsc= IMP.container.ListSingletonContainer(m)
    ps=IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)
    dsc.add_particles(ps)

    dpc = IMP.container.ClosePairContainer(dsc, 30.0, 0.0)
    pair_filter = IMP.atom.StereochemistryPairFilter()
    pair_filter.set_bonds(bonds)
    pair_filter.set_angles(angles)
    pair_filter.set_dihedrals(dihedrals)
    dpc.add_pair_filter(pair_filter)

    IMP.membrane.add_dope_score_data(prot)
    dps= IMP.membrane.DopePairScore(100.0)
    d=   IMP.container.PairsRestraint(dps, dpc)
    m.add_restraint(d)

# Here starts the real job...
IMP.set_log_level(IMP.VERBOSE)

print "creating representation"
(m,prot)=create_representation()

print "creating DOPE score function"
add_DOPE(m,prot)

print "update model"
m.update()
