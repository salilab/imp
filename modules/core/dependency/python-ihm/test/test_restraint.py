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

    def test_distance_restraint(self):
        """Test DistanceRestraint class"""
        r = ihm.restraint.DistanceRestraint() # does nothing

    def test_harmonic_distance_restraint(self):
        """Test HarmonicDistanceRestraint class"""
        r = ihm.restraint.HarmonicDistanceRestraint(42.0)
        self.assertAlmostEqual(r.distance, 42.0, places=1)
        self.assertEqual(r.restraint_type, "harmonic")

    def test_upper_bound_distance_restraint(self):
        """Test UpperBoundDistanceRestraint class"""
        r = ihm.restraint.UpperBoundDistanceRestraint(42.0)
        self.assertAlmostEqual(r.distance, 42.0, places=1)
        self.assertEqual(r.restraint_type, "upper bound")

    def test_lower_bound_distance_restraint(self):
        """Test LowerBoundDistanceRestraint class"""
        r = ihm.restraint.LowerBoundDistanceRestraint(42.0)
        self.assertAlmostEqual(r.distance, 42.0, places=1)
        self.assertEqual(r.restraint_type, "lower bound")

    def test_cross_link_restraint(self):
        """Test CrossLinkRestraint class"""
        f = ihm.restraint.CrossLinkRestraint(dataset='foo', linker_type='DSS')
        self.assertEqual(f.dataset, 'foo')
        self.assertEqual(f.linker_type, 'DSS')
        self.assertEqual(f.experimental_cross_links, [])

    def test_experimental_cross_link(self):
        """Test ExperimentalCrossLink class"""
        f = ihm.restraint.ExperimentalCrossLink('res1', 'res2')
        self.assertEqual(f.residue1, 'res1')
        self.assertEqual(f.residue2, 'res2')

    def test_cross_link(self):
        """Test CrossLink class"""
        f = ihm.restraint.CrossLink() # does nothing

    def test_residue_cross_link(self):
        """Test ResidueCrossLink class"""
        f = ihm.restraint.ResidueCrossLink(experimental_cross_link='ex',
                asym1='asym1', asym2='asym2', distance='dist')
        self.assertEqual(f.granularity, 'by-residue')
        self.assertEqual(f.atom1, None)
        self.assertEqual(f.atom2, None)
        self.assertEqual(f.asym1, 'asym1')
        self.assertEqual(f.asym2, 'asym2')

    def test_feature_cross_link(self):
        """Test FeatureCrossLink class"""
        f = ihm.restraint.FeatureCrossLink(experimental_cross_link='ex',
                asym1='asym1', asym2='asym2', distance='dist')
        self.assertEqual(f.granularity, 'by-feature')
        self.assertEqual(f.atom1, None)
        self.assertEqual(f.atom2, None)
        self.assertEqual(f.asym1, 'asym1')
        self.assertEqual(f.asym2, 'asym2')

    def test_atom_cross_link(self):
        """Test AtomCrossLink class"""
        f = ihm.restraint.AtomCrossLink(experimental_cross_link='ex',
                asym1='asym1', asym2='asym2', atom1='C', atom2='N',
                distance='dist')
        self.assertEqual(f.granularity, 'by-atom')
        self.assertEqual(f.atom1, 'C')
        self.assertEqual(f.atom2, 'N')
        self.assertEqual(f.asym1, 'asym1')
        self.assertEqual(f.asym2, 'asym2')


if __name__ == '__main__':
    unittest.main()
