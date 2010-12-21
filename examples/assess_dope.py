import IMP
import IMP.atom
import IMP.container
import IMP.membrane

class SameResidueFilter(IMP.PairFilter):
    def __init__(self):
        IMP.PairFilter.__init__(self)
    def get_contains_particle_pair(self, pp):
        diff= pp[0].get_value(IMP.IntKey("num"))-pp[1].get_value(IMP.IntKey("num"))
        if diff==0:
            return True
        return False
    def get_input_particles(self, p):
        return [p]
    def get_input_containers(self, p):
        return []
    def do_show(self, out):
        pass

def create_representation():
    m=IMP.Model()
    mp0= IMP.atom.read_pdb('1fdx.B99990001.pdb', m, IMP.atom.NonWaterNonHydrogenPDBSelector())
    prot=IMP.atom.get_by_type(mp0, IMP.atom.CHAIN_TYPE)[0]
    return (m, prot)

def add_DOPE(m, prot):
    ps=IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)
    dsc= IMP.container.ListSingletonContainer(m)
    for p in ps:
        p.add_attribute(IMP.IntKey("num"), IMP.atom.get_residue(IMP.atom.Atom(p)).get_index())
    dsc.add_particles(ps)
    dpc = IMP.container.ClosePairContainer(dsc, 15.0, 0.0)
# exclude pairs of atoms belonging to the same residue
# for consistency with MODELLER DOPE score
    f= SameResidueFilter()
    dpc.add_pair_filter(f)
    IMP.membrane.add_dope_score_data(prot)
    dps= IMP.membrane.DopePairScore(15.0)
#    dps= IMP.membrane.DopePairScore(15.0, IMP.membrane.get_data_path("dope_scorehr.lib"))
    d=   IMP.container.PairsRestraint(dps, dpc)
    m.add_restraint(d)

# Here starts the real job...
#IMP.set_log_level(IMP.VERBOSE)

print "creating representation"
(m,prot)=create_representation()

print "creating DOPE score function"
add_DOPE(m,prot)

print "update model"
m.update()
