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
        self.assertAlmostEqual(r.distance_lower_limit, 42.0, places=1)
        self.assertAlmostEqual(r.distance_upper_limit, 42.0, places=1)
        self.assertEqual(r.restraint_type, "harmonic")

    def test_upper_bound_distance_restraint(self):
        """Test UpperBoundDistanceRestraint class"""
        r = ihm.restraint.UpperBoundDistanceRestraint(42.0)
        self.assertAlmostEqual(r.distance, 42.0, places=1)
        self.assertEqual(r.distance_lower_limit, None)
        self.assertAlmostEqual(r.distance_upper_limit, 42.0, places=1)
        self.assertEqual(r.restraint_type, "upper bound")

    def test_lower_bound_distance_restraint(self):
        """Test LowerBoundDistanceRestraint class"""
        r = ihm.restraint.LowerBoundDistanceRestraint(42.0)
        self.assertAlmostEqual(r.distance, 42.0, places=1)
        self.assertAlmostEqual(r.distance_lower_limit, 42.0, places=1)
        self.assertEqual(r.distance_upper_limit, None)
        self.assertEqual(r.restraint_type, "lower bound")

    def test_lower_upper_bound_distance_restraint(self):
        """Test LowerUpperBoundDistanceRestraint class"""
        r = ihm.restraint.LowerUpperBoundDistanceRestraint(20.0, 30.0)
        self.assertAlmostEqual(r.distance_lower_limit, 20.0, places=1)
        self.assertAlmostEqual(r.distance_upper_limit, 30.0, places=1)
        self.assertEqual(r.restraint_type, "lower and upper bound")

    def test_cross_link_restraint(self):
        """Test CrossLinkRestraint class"""
        dss = ihm.ChemDescriptor('DSS')
        f = ihm.restraint.CrossLinkRestraint(dataset='foo', linker=dss)
        self.assertEqual(f.dataset, 'foo')
        self.assertEqual(f.linker, dss)
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

    def test_feature(self):
        """Test Feature base class"""
        f = ihm.restraint.Feature() # does nothing

    def test_residue_feature(self):
        """Test ResidueFeature class"""
        e = ihm.Entity('AHCDAH')
        a = ihm.AsymUnit(e)
        f = ihm.restraint.ResidueFeature(ranges=[])
        self.assertEqual(f._get_entity_type(), None)

        # No ranges - type is 'residue'
        self.assertEqual(f.type, 'residue')

        # All ranges are individual residues
        f.ranges.append(a(1,1))
        f.ranges.append(a(2,2))
        self.assertEqual(f.type, 'residue')

        # At least one range is a true range
        f.ranges.append(a(3,4))
        self.assertEqual(f.type, 'residue range')


    def test_geometric_restraint(self):
        """Test GeometricRestraint class"""
        f = ihm.restraint.GeometricRestraint(
                dataset='foo', geometric_object='geom', feature='feat',
                distance='dist')
        self.assertEqual(f.dataset, 'foo')
        self.assertEqual(f.object_characteristic, 'other')
        self.assertEqual(f.assembly, None)

    def test_center_geometric_restraint(self):
        """Test CenterGeometricRestraint class"""
        f = ihm.restraint.CenterGeometricRestraint(
                dataset='foo', geometric_object='geom', feature='feat',
                distance='dist')
        self.assertEqual(f.dataset, 'foo')
        self.assertEqual(f.object_characteristic, 'center')
        self.assertEqual(f.assembly, None)

    def test_inner_surface_geometric_restraint(self):
        """Test InnerSurfaceGeometricRestraint class"""
        f = ihm.restraint.InnerSurfaceGeometricRestraint(
                dataset='foo', geometric_object='geom', feature='feat',
                distance='dist')
        self.assertEqual(f.dataset, 'foo')
        self.assertEqual(f.object_characteristic, 'inner surface')
        self.assertEqual(f.assembly, None)

    def test_outer_surface_geometric_restraint(self):
        """Test OuterSurfaceGeometricRestraint class"""
        f = ihm.restraint.OuterSurfaceGeometricRestraint(
                dataset='foo', geometric_object='geom', feature='feat',
                distance='dist')
        self.assertEqual(f.dataset, 'foo')
        self.assertEqual(f.object_characteristic, 'outer surface')
        self.assertEqual(f.assembly, None)

    def test_derived_distance_restraint(self):
        """Test DerivedDistanceRestraint class"""
        f = ihm.restraint.DerivedDistanceRestraint(
                dataset='foo', feature1='feat1', feature2='feat2',
                distance='dist')
        self.assertEqual(f.dataset, 'foo')
        self.assertEqual(f.assembly, None)


if __name__ == '__main__':
    unittest.main()
