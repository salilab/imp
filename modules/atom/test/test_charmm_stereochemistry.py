import IMP
import IMP.test
import IMP.atom


def get_all_atoms(pdb):
    atoms = {}
    residues = IMP.atom.get_by_type(pdb, IMP.atom.RESIDUE_TYPE)
    for ni, res in enumerate(residues):
        resatoms = IMP.atom.get_by_type(res, IMP.atom.ATOM_TYPE)
        for a in resatoms:
            aid = '%d:' % (ni + 1) \
                  + IMP.atom.Atom(a).get_atom_type().get_string()
            atoms[aid] = a.get_particle()
    return atoms

class Tests(IMP.test.TestCase):

    """Test CHARMM stereochemistry restraint"""

    def setup_restraint(self):
        m = IMP.kernel.Model()
        pdb = IMP.atom.read_pdb(
            self.get_input_file_name('charmm_type_test.pdb'), m)
        ff = IMP.atom.get_heavy_atom_CHARMM_parameters()
        topology = ff.create_topology(pdb)
        topology.apply_default_patches()
        topology.setup_hierarchy(pdb)
        r = IMP.atom.CHARMMStereochemistryRestraint(pdb, topology)
        m.add_restraint(r)
        return r, m, pdb

    def assertContainsPair(self, pf, atoms, a1, a2):
        self.assertEquals(pf.get_value([atoms[a1], atoms[a2]]),
                          True)

    def test_score(self):
        """Test CHARMMStereochemistryRestraint::evaluate()"""
        r, m, pdb = self.setup_restraint()
        score = pdb.get_model().evaluate(False)
        self.assertAlmostEqual(score, 2.90562, delta=0.02)

    def test_pair_filter(self):
        """Test CHARMMStereochemistryRestraint pair filter"""
        r, m, pdb = self.setup_restraint()
        pf = r.get_pair_filter()
        atoms = get_all_atoms(pdb)
        # Bonds (1-2 pairs)
        self.assertContainsPair(pf, atoms, '1:N', '1:CA')
        self.assertContainsPair(pf, atoms, '1:C', '2:N')
        # Angles (1-3 pairs)
        self.assertContainsPair(pf, atoms, '1:CA', '1:OG')
        # Dihedrals (1-4 pairs)
        self.assertContainsPair(pf, atoms, '1:N', '1:OG')

    def test_get_inputs(self):
        """Test CHARMMStereochemistryRestraint get_inputs()"""
        r, m, pdb = self.setup_restraint()
        ps = r.get_inputs()
        self.assertEqual(len(ps), 190)


class SelectionTests(IMP.test.TestCase):

    """Test CHARMM stereochemistry restraint with seletion"""

    def setup_restraint(self):
        m = IMP.kernel.Model()
        pdb = IMP.atom.read_pdb(
            self.get_input_file_name('charmm_type_test.pdb'), m)
        ff = IMP.atom.get_heavy_atom_CHARMM_parameters()
        topology = ff.create_topology(pdb)
        topology.apply_default_patches()
        topology.setup_hierarchy(pdb)
        sel = IMP.atom.Selection(pdb,residue_index=2)
        r = IMP.atom.CHARMMStereochemistryRestraint(pdb, topology,
                                                    sel.get_selected_particles())
        m.add_restraint(r)
        return r, m, pdb

    def assertContainsPair(self, pf, atoms, a1, a2):
        self.assertEquals(pf.get_value([atoms[a1], atoms[a2]]),
                          True)
    def assertDoesNotContainsPair(self, pf, atoms, a1, a2):
        self.assertEquals(pf.get_value([atoms[a1], atoms[a2]]),
                          False)

    def test_limited_pair_filter(self):
        """Test CHARMMStereochemistryRestraint pair filter"""
        r, m, pdb = self.setup_restraint()
        pf = r.get_pair_filter()
        atoms = get_all_atoms(pdb)
        # Bonds (1-2 pairs)
        self.assertContainsPair(pf, atoms, '1:N', '1:CA')
        self.assertDoesNotContainsPair(pf, atoms, '1:C', '2:N')
        # Angles (1-3 pairs)
        self.assertContainsPair(pf, atoms, '1:CA', '1:OG')
        # Dihedrals (1-4 pairs)
        self.assertContainsPair(pf, atoms, '1:N', '1:OG')

    def test_limited_get_inputs(self):
        """Test CHARMMStereochemistryRestraint get_inputs()"""
        r, m, pdb = self.setup_restraint()
        ps = r.get_inputs()
        self.assertEqual(len(ps), 75)

if __name__ == '__main__':
    IMP.test.main()
