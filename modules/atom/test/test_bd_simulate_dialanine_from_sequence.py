import IMP
import IMP.atom
import IMP.core
import IMP.rmf
import IMP.test
import RMF

##### CONSTANT ####
N_ALANINES = 2
# Radii by atomic weight of various elements
radius_by_element_id = {
    1: 1.2, # H
    6: 1.7, # C
    7: 1.55, # N
    8: 1.52, # O
}
BD_STEP_SIZE_FS = 1.0
RMF_FNAME_PREFIX = "dialanine.rmf"
BD_CYCLES = 100
RMF_PERIOD = 10

##### AUXILIARY FUNCTIOONS ####
def add_poly_ALA_to_fragment(fragment, num):
    assert (IMP.atom.Fragment.get_is_setup(fragment))
    m = fragment.get_model()
    for i in range(num):
        p = IMP.Particle(m)
        r = IMP.atom.Residue.setup_particle(p, IMP.atom.ALA)
        fragment.add_child(r)

def create_hierarchy_root(m):
    p = IMP.Particle(m)
    h = IMP.atom.Hierarchy.setup_particle(p)
    return h

def create_fragment(m, num):
    p = IMP.Particle(m)
    f = IMP.atom.Fragment.setup_particle(p)
    add_poly_ALA_to_fragment(f, num)
    f.set_residue_indexes([i+1 for i in range(num)])
    return f

def get_num_res(t):
    return [s.get_number_of_residues() for s in t.get_segments()]

def create_topology():
    m = IMP.Model()
    ff = IMP.atom.get_heavy_atom_CHARMM_parameters()
    root = create_hierarchy_root(m)
    c1 = IMP.atom.Chain.setup_particle(IMP.Particle(m), "A")
    root.add_child(c1)
    c1.add_child(create_fragment(m, N_ALANINES))
    t = ff.create_topology(root)
#    assert(get_num_res(t) == [N_ALANINES])
    return t

def get_scoring_function(protein, topology, is_non_bonded=True):
    force_field = topology.get_parameters()
    restraints = []
    restraints.append(IMP.atom.CHARMMStereochemistryRestraint(protein, topology))
    if is_non_bonded:
        force_field.add_radii(protein)
        force_field.add_well_depths(protein)
        atoms = IMP.atom.get_by_type(protein, IMP.atom.ATOM_TYPE)
        container = IMP.container.ListSingletonContainer(m, atoms)
        cpc = IMP.container.ClosePairContainer(cont, 4.0)
        cpc.add_pair_filter(r.get_pair_filter())
        sf = IMP.atom.ForceSwitch(6.0, 7.0)
        ljps = IMP.atom.LennardJonesPairScore(sf)
        restraints.append(IMP.container.PairsRestraint(ljps, cpc))
    scoring_function = IMP.core.RestraintsScoringFunction(restraints)
    return scoring_function

##### Tests #####
class Tests(IMP.test.TestCase):
    def _create_dialanine_hierarchy_and_topology(self, model):
        topology = create_topology()
        protein = topology.create_hierarchy(model)
        topology.apply_default_patches()
        topology.setup_hierarchy(protein)
        topology.add_coordinates (protein)
        atoms = IMP.atom.get_by_type(protein, IMP.atom.ATOM_TYPE)
        for atom in atoms:
            self.assertTrue(IMP.atom.Atom.get_is_setup(atom))
            element_id = IMP.atom.Atom(atom).get_element()
            atom_radius = radius_by_element_id[element_id]
            xyzr = IMP.core.XYZR.setup_particle(atom)
            xyzr.set_radius(atom_radius)
            if not IMP.atom.Diffusion.get_is_setup(atom):
                IMP.atom.Diffusion.setup_particle(atom)
        return protein, topology

    def test_bd_simulate_dialanine_from_sequence(self):
        model = IMP.Model()
        protein, topology = self._create_dialanine_hierarchy_and_topology(model)
        sf = get_scoring_function(protein, topology, False)
        bd = IMP.atom.BrownianDynamics(model)
        bd.set_maximum_time_step(BD_STEP_SIZE_FS)
        bd.set_scoring_function(sf)
        rmf_fname = self.get_tmp_file_name(RMF_FNAME_PREFIX)
        rmf = RMF.create_rmf_file(rmf_fname)
        IMP.rmf.add_hierarchy(rmf, protein.get_children()[0])
        os = IMP.rmf.SaveOptimizerState(model, rmf)
        os.set_period(RMF_PERIOD)
        bd.add_optimizer_state(os)
        self.assertAlmostEqual(sf.evaluate(False), 9.224793596016248)
        print("Atoms before optimizing:")
        atoms = IMP.atom.get_by_type(protein, IMP.atom.ATOM_TYPE)
        print(atoms)
        print("Optimizing")
        bd.optimize(BD_CYCLES)
        print(f"Atoms after optimizing for {BD_CYCLES} cycles:")
        print(atoms)
        print("Final score = {:.2f}".format(sf.evaluate(False)))


if __name__ == '__main__':
    IMP.test.main()
