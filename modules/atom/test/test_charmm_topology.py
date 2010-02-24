import unittest
import IMP
import IMP.test
import IMP.atom

def _make_test_atom():
    at = IMP.atom.CHARMMAtom('CA')
    at.set_charmm_type('CT1')
    at.set_charge(0.8)
    return at

class CHARMMTopologyTests(IMP.test.TestCase):
    """Test CHARMM topology classes"""

    def test_atom(self):
        """Check creation and methods of CHARMMAtom"""
        at = _make_test_atom()
        self.assertEqual(at.get_name(), 'CA')
        self.assertEqual(at.get_charmm_type(), 'CT1')
        self.assertInTolerance(at.get_charge(), 0.8, 1e-6)

    def test_add_get_atom(self):
        """Check add/get atom from CHARMM residues"""
        res = IMP.atom.CHARMMIdealResidueTopology('FOO')
        self.assertEqual(res.get_name(), 'FOO')
        self.assertRaises(IMP.ValueException, res.get_atom, 'CA')
        at = _make_test_atom()
        res.add_atom(at)
        self.assertEqual(res.get_atom('CA').get_charmm_type(), 'CT1')
        # Modifying our copy after it was inserted should have no effect
        at.set_charmm_type('CT2')
        self.assertEqual(res.get_atom('CA').get_charmm_type(), 'CT1')
        # Modifying the residue copy should work, however
        at = res.get_atom('CA')
        at.set_charmm_type('CT2')
        self.assertEqual(res.get_atom('CA').get_charmm_type(), 'CT2')

    def test_delete_atom(self):
        """Check delete atom from CHARMM residues"""
        res = IMP.atom.CHARMMIdealResidueTopology('FOO')
        at = _make_test_atom()
        res.add_atom(at)
        self.assertRaises(IMP.ValueException, res.delete_atom, 'CB')
        res.delete_atom('CA')
        self.assertRaises(IMP.ValueException, res.get_atom, 'CA')

    def test_default_patches(self):
        """Check default patches of CHARMM residues"""
        res = IMP.atom.CHARMMIdealResidueTopology('FOO')
        self.assertEqual(res.get_default_first_patch(), '')
        self.assertEqual(res.get_default_last_patch(), '')
        res.set_default_first_patch('NTER')
        res.set_default_last_patch('CTER')
        self.assertEqual(res.get_default_first_patch(), 'NTER')
        self.assertEqual(res.get_default_last_patch(), 'CTER')

    def test_bond_endpoint(self):
        """Check CHARMM bond endpoint class"""
        e = IMP.atom.CHARMMBondEndpoint('CA')
        self.assertEqual(e.get_atom_name(), 'CA')

    def test_bond(self):
        """Check CHARMM bond class"""
        bond = IMP.atom.CHARMMBond2(IMP.Strings(['CA', 'CB']))
        self.assertEqual(bond.contains_atom('CA'), True)
        self.assertEqual(bond.contains_atom('N'), False)

    def test_add_bond(self):
        """Check addition of bonds/angles/dihedrals/impropers"""
        res = IMP.atom.CHARMMIdealResidueTopology('FOO')
        atoms = IMP.Strings(['CA', 'CB'])
        # Test construction from strings or from CHARMMBond object
        res.add_bond(atoms)
        res.add_bond(IMP.atom.CHARMMBond2(atoms))
        self.assertEqual(res.get_number_of_bonds(), 2)
        atoms = IMP.Strings(['CA', 'CB', 'N'])
        res.add_angle(IMP.atom.CHARMMBond3(atoms))
        self.assertEqual(res.get_number_of_angles(), 1)
        atoms = IMP.Strings(['CA', 'CB', 'N', 'O'])
        res.add_dihedral(IMP.atom.CHARMMBond4(atoms))
        self.assertEqual(res.get_number_of_dihedrals(), 1)
        res.add_improper(atoms)
        self.assertEqual(res.get_number_of_impropers(), 1)
        self.assertEqual(res.get_bond(0).contains_atom('CA'), True)
        self.assertEqual(res.get_angle(0).contains_atom('N'), True)
        self.assertEqual(res.get_dihedral(0).contains_atom('O'), True)
        self.assertEqual(res.get_improper(0).contains_atom('O'), True)

    def test_patch(self):
        """Check the CHARMM patch class"""
        patch = IMP.atom.CHARMMPatch('PFOO')
        self.assertEqual(patch.get_name(), 'PFOO')
        patch.add_deleted_atom('CA')

    def test_forcefield_add_get(self):
        """Test adding/getting patches and residues to/from forcefields"""
        ff = IMP.atom.CharmmParameters(IMP.atom.get_data_path("top.lib"))
        self.assertRaises(IMP.ValueException, ff.get_residue_topology, 'FOO')
        self.assertRaises(IMP.ValueException, ff.get_patch, 'PFOO')
        patch = IMP.atom.CHARMMPatch('PFOO')
        res = IMP.atom.CHARMMIdealResidueTopology('FOO')
        ff.add_patch(patch)
        ff.add_residue_topology(res)
        ff.get_residue_topology('FOO')
        ff.get_patch('PFOO')

    def test_forcefield_read(self):
        """Test read of topology from files"""
        ff = IMP.atom.CharmmParameters(IMP.atom.get_data_path("top.lib"))
        self.assertRaises(ValueError, ff.get_residue_topology, 'CTER')
        res = ff.get_residue_topology('CYS')
        self.assertEqual(res.get_number_of_bonds(), 11)
        self.assertEqual(res.get_number_of_angles(), 0)
        self.assertEqual(res.get_number_of_dihedrals(), 0)
        self.assertEqual(res.get_number_of_impropers(), 3)
        for (name, typ, charge) in [('N', 'NH1', -0.47), ('H', 'H', 0.31),
                                    ('CA', 'CT1', 0.07), ('HA', 'HB', 0.09),
                                    ('CB', 'CT2', -0.11), ('HB3', 'HA', 0.09),
                                    ('HB2', 'HA', 0.09), ('SG', 'S', -0.23),
                                    ('HG', 'HS', 0.16), ('C', 'C',  0.51),
                                    ('O', 'O',  -0.51)]:
            at = res.get_atom(name)
            self.assertEqual(at.get_charmm_type(), typ)
            self.assertInTolerance(at.get_charge(), charge, 1e-3)

        self.assertRaises(ValueError, ff.get_patch, 'CYS')
        patch = ff.get_patch('CTER')
        for (name, typ, charge) in [('C', 'CC', 0.34), ('O', 'OC', -0.67),
                                    ('OXT', 'OC', -0.67)]:
            at = patch.get_atom(name)
            self.assertEqual(at.get_charmm_type(), typ)
            self.assertInTolerance(at.get_charge(), charge, 1e-3)

        for (name, first, last) in [('ALA', 'NTER', 'CTER'),
                                    ('GLY', 'GLYP', 'CTER'),
                                    ('UNK', 'GLYP', 'CTER'),
                                    ('PRO', 'PROP', 'CTER'),
                                    ('TIP3', '', ''),
                                    ('SOD', '', '')]:
            res = ff.get_residue_topology(name)
            self.assertEqual(res.get_default_first_patch(), first)
            self.assertEqual(res.get_default_last_patch(), last)

    def test_residue_topology(self):
        """Test CHARMM residue topology objects"""
        ideal = IMP.atom.CHARMMIdealResidueTopology('ALA')
        at = _make_test_atom()
        ideal.add_atom(at)
        res = IMP.atom.CHARMMResidueTopology(ideal)
        self.assertEqual(res.get_atom('CA').get_charmm_type(), 'CT1')
        self.assertEqual(res.get_patched(), False)
        res.set_patched(True)
        self.assertEqual(res.get_patched(), True)

    def test_patching(self):
        """Test application of patches"""
        ff = IMP.atom.CharmmParameters(IMP.atom.get_data_path("top.lib"))
        patch = ff.get_patch('CTER')
        res = IMP.atom.CHARMMResidueTopology(ff.get_residue_topology('ALA'))
        self.assertEqual(res.get_atom('C').get_charmm_type(), 'C')
        patch.apply(res)
        # Patch should change atom type of existing atoms
        self.assertEqual(res.get_atom('C').get_charmm_type(), 'CC')
        # Should also add new atoms
        self.assertEqual(res.get_atom('OXT').get_charmm_type(), 'OC')
        self.assertEqual(res.get_patched(), True)
        # Repeated patching should not be possible
        self.assertRaises(IMP.ValueException, patch.apply, res)

        # Patches should delete atoms
        patch = ff.get_patch('TP1A')
        res = IMP.atom.CHARMMResidueTopology(ff.get_residue_topology('TYR'))
        self.assertEqual(res.get_atom('CB').get_charmm_type(), 'CT2')
        patch.apply(res)
        self.assertRaises(IMP.ValueException, res.get_atom, 'CB')

if __name__ == '__main__':
    unittest.main()
