import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.algebra

class CHARMMParametersTests(IMP.test.TestCase):
    """Test the CharmmParameters class"""

    def test_bond_parameters(self):
        """Check extraction of bond parameters"""
        p = IMP.atom.CharmmParameters(IMP.atom.get_data_path('top.lib'),
                                      IMP.atom.get_data_path('par.lib'))
        self.assertEqual(p.get_bond_parameters('garbage', 'CT2'), None)
        for types in [('CT1', 'CT2'), ('CT2', 'CT1')]:
            bond = p.get_bond_parameters(*types)
            self.assertInTolerance(bond.force_constant, 222.500, 1e-4)
            self.assertInTolerance(bond.mean, 1.5380, 1e-5)

if __name__ == '__main__':
    unittest.main()
