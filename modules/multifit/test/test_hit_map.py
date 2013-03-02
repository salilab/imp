import IMP.em
import IMP.test
import IMP.multifit
import IMP.algebra
import IMP.atom
import IMP.core

class Tests(IMP.test.TestCase):
    """Class to test hit map generation"""

    def read_data(self):
        model = IMP.Model()
        mrw = IMP.em.MRCReaderWriter()
        scene = IMP.em.read_map(self.get_input_file_name("3points.mrc"), mrw)
        scene.get_header_writable().set_resolution(8.)
        mp = IMP.atom.read_pdb(self.open_input_file("3points.pdb"),
                               model, IMP.atom.CAlphaPDBSelector())
        rb = IMP.atom.create_rigid_body(mp)
        ref = IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
        return model, rb, ref, scene

    def test_good_hit_map(self):
        """Test hit map generation with a good fit"""
        model, rb, ref, scene = self.read_data()

        # PDB is already fitted on the map, so an identity transformation
        # is a good fit
        identity_fit = IMP.multifit.FittingSolutionRecord()
        identity_fit.set_fit_transformation(
                IMP.algebra.get_identity_transformation_3d())

        hits = IMP.multifit.create_hit_map(rb, ref, [identity_fit], scene)
        self.assertAlmostEqual(hits.get_max_value(), 1.0, delta=1e-4)

    def test_bad_hit_map(self):
        """Test hit map generation with a bad fit"""
        model, rb, ref, scene = self.read_data()

        # Translate the PDB far away from the map
        bad_fit = IMP.multifit.FittingSolutionRecord()
        bad_fit.set_fit_transformation(
           IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),
                                        IMP.algebra.Vector3D(90.,90.,90.)))

        hits = IMP.multifit.create_hit_map(rb, ref, [bad_fit], scene)
        self.assertAlmostEqual(hits.get_max_value(), 0.0, delta=1e-4)

if __name__ == '__main__':
    IMP.test.main()
