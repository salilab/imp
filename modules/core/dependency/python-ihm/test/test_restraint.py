import utils
import os
import unittest
import sys

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.restraint

class Tests(unittest.TestCase):

    def test_restraint(self):
        """Test Restraint base class"""
        r = ihm.restraint.Restraint() # does nothing

    def test_em3d_restraint_fit(self):
        """Test EM3DRestraintFit class"""
        f = ihm.restraint.EM3DRestraintFit(0.4)
        self.assertAlmostEqual(f.cross_correlation_coefficient, 0.4, places=1)

    def test_em3d_restraint(self):
        """Test EM3DRestraint class"""
        f = ihm.restraint.EM3DRestraint(dataset='foo', assembly='bar')
        self.assertEqual(f.dataset, 'foo')
        self.assertEqual(f.assembly, 'bar')
        self.assertEqual(f.fits, {})

    def test_sas_restraint_fit(self):
        """Test SASRestraintFit class"""
        f = ihm.restraint.SASRestraintFit(0.4)
        self.assertAlmostEqual(f.chi_value, 0.4, places=1)

    def test_sas_restraint(self):
        """Test SASRestraint class"""
        f = ihm.restraint.SASRestraint(dataset='foo', assembly='bar')
        self.assertEqual(f.dataset, 'foo')
        self.assertEqual(f.assembly, 'bar')
        self.assertEqual(f.fits, {})

    def test_em2d_restraint_fit(self):
        """Test EM2DRestraintFit class"""
        f = ihm.restraint.EM2DRestraintFit(0.4)
        self.assertAlmostEqual(f.cross_correlation_coefficient, 0.4, places=1)
        self.assertEqual(f.rot_matrix, None)
        self.assertEqual(f.tr_vector, None)

    def test_em2d_restraint(self):
        """Test EM2DRestraint class"""
        f = ihm.restraint.EM2DRestraint(dataset='foo', assembly='bar')
        self.assertEqual(f.dataset, 'foo')
        self.assertEqual(f.assembly, 'bar')
        self.assertEqual(f.fits, {})


if __name__ == '__main__':
    unittest.main()
