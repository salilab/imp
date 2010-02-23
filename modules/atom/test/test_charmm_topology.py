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
        res = IMP.atom.CHARMMIdealResidueTopology()
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
        res = IMP.atom.CHARMMIdealResidueTopology()
        at = _make_test_atom()
        res.add_atom(at)
        self.assertRaises(IMP.ValueException, res.delete_atom, 'CB')
        res.delete_atom('CA')
        self.assertRaises(IMP.ValueException, res.get_atom, 'CA')

if __name__ == '__main__':
    unittest.main()
