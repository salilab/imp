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

    def test_add_bond(self):
        """Check addition of bonds/angles/dihedrals/impropers"""
        res = IMP.atom.CHARMMIdealResidueTopology('FOO')
        atoms = IMP.Strings(['CA', 'CB'])
        res.add_bond(atoms)
        atoms = IMP.Strings(['CA', 'CB', 'N'])
        res.add_angle(atoms)
        atoms = IMP.Strings(['CA', 'CB', 'N', 'O'])
        res.add_dihedral(atoms)
        res.add_improper(atoms)

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

if __name__ == '__main__':
    unittest.main()
