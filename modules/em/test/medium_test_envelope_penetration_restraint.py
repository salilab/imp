from __future__ import print_function
import IMP
import IMP.em
import IMP.test
import IMP.core


class Tests(IMP.test.TestCase):

    """Class to test EM correlation restraint"""

    def _create_density_map(self, res, apix):
        self.dmap = IMP.em.particles2density(self.ps,
                                             res, apix)
        self.dmap.show()
        nr = len(IMP.atom.get_by_type(self.mh, IMP.atom.RESIDUE_TYPE))
        self.thr = IMP.em.get_threshold_for_approximate_mass(
            self.dmap, 2 * IMP.atom.get_mass_from_number_of_residues(nr))
        print("threshold:", self.thr)

    def _load_particles(self):
        self.mh = IMP.atom.read_pdb(
            self.get_input_file_name('1tdx.pdb'),
            self.mdl)
        self.ps = IMP.core.get_leaves(self.mh)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.mdl = IMP.Model()
        self._load_particles()
        self._create_density_map(10, 1.5)
        self.epr = IMP.em.EnvelopePenetrationRestraint(
            self.ps, self.dmap, self.thr)
        self.sf = IMP.core.RestraintsScoringFunction([self.epr])

    def test_evaluate(self):
        """Check the restraint is being evaluated"""
        score = self.sf.evaluate(False)
        self.assertAlmostEqual(score, 0.0, delta=0.1)

    def test_penetration(self):
        trans = IMP.algebra.Vector3D(5, 5, 5)
        t = IMP.algebra.Transformation3D(
            IMP.algebra.get_identity_rotation_3d(),
            trans)
        xyz = IMP.core.XYZs(self.ps)
        for x in xyz:
            x.set_coordinates(t.get_transformed(x.get_coordinates()))
        score = self.sf.evaluate(False)
        self.assertAlmostEqual(score, 59.0, delta=1)


if __name__ == '__main__':
    IMP.test.main()
