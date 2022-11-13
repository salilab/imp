import IMP
import IMP.core
import IMP.atom
import IMP.rmf
import RMF

# Radii by atomic weight of various elements
radius_by_element_id = { 
    1: 1.2, # H
    6: 1.7, # C 
    7: 1.55, # N
    8: 1.52, # O
    21988: 1.7,
    21869: 1.7,
    21856: 1.7
}
RMF_FNAME = "dialanine.rmf"
BD_CYCLES = 100

def add_atom_types_for_capped_amino_acids():
    # Add atom types referenced by ACE and CT3 patches that IMP
    # doesn't know about
    elements = IMP.atom.get_element_table()
    IMP.atom.add_atom_type('CAY', elements.get_element('C'))
    IMP.atom.add_atom_type('CY', elements.get_element('C'))
    IMP.atom.add_atom_type('OY', elements.get_element('O'))
    IMP.atom.add_atom_type('CAT', elements.get_element('C'))

def create_dialanine_topology():
    add_atom_types_for_capped_amino_acids()
    ff = IMP.atom.get_heavy_atom_CHARMM_parameters()
    st = IMP.atom.CHARMMSegmentTopology()
    ideal_ala = ff.get_residue_topology(IMP.atom.ALA)
    N_terminal_res = IMP.atom.CHARMMResidueTopology(ideal_ala)
    ace = ff.get_patch("ACE")
    ace.apply(N_terminal_res)
    N_terminal_res.set_patched(False)
    #C_terminal_res = IMP.atom.CHARMMResidueTopology(ideal_ala)
    ct3 = ff.get_patch("CT3")
    ct3.apply(N_terminal_res)
    st.add_residue(N_terminal_res)
#    st.add_residue(C_terminal_res)
    t = IMP.atom.CHARMMTopology(ff)
    t.add_segment(st)
    Debug = True
    if Debug:
        # Make a Hierarchy using this topology
        m = IMP.Model()
        h = t.create_hierarchy(m)
        IMP.atom.show(h)
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
    def test_bd_simulate_dialanine_capped():
        topology = create_dialanine_topology()
        model = IMP.Model()
        protein = topology.create_hierarchy(model)
        #    topology.apply_default_patches()
        topology.setup_hierarchy(protein)
        topology.add_coordinates (protein)
        atoms = IMP.atom.get_by_type(protein, IMP.atom.ATOM_TYPE)
        for atom in atoms:
            assert(IMP.atom.Atom.get_is_setup(atom))
            element_id = IMP.atom.Atom(atom).get_element()
            atom_radius = radius_by_element_id[element_id]
            xyzr = IMP.core.XYZR.setup_particle(atom) 
            xyzr.set_radius(atom_radius)
            if not IMP.atom.Diffusion.get_is_setup(atom):
                IMP.atom.Diffusion.setup_particle(atom)
        sf = get_scoring_function(protein, topology, False)
        bd = IMP.atom.BrownianDynamics(model)
        bd.set_maximum_time_step(1)
        bd.set_scoring_function(sf)
        rmf_fname = self.get_tmp_file_name(RMF_FNAME)
        print(f"RMF file: {rmf_fname"})
        rmf = RMF.create_rmf_file(rmf_fname)
        IMP.rmf.add_hierarchy(rmf, protein.get_children()[0])
        os = IMP.rmf.SaveOptimizerState(model, rmf)
        os.set_period(10)
        bd.add_optimizer_state(os)
        print(atoms)
        print(sf.evaluate(False))
        print("Optimizing")
        bd.optimize(BD_CYCLES)
        print("Atoms after optimizing for %d cycles:" % BD_CYCLES)
        print(atoms)
        print("Final score = %.2f" % sf.evaluate(False))
        assertAlmostEqual(sf.evaluate(False), 9.85)
    

if __name__ == '__main__':
    IMP.test.main()
