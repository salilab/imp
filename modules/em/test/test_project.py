import IMP
import IMP.test
import IMP.em
import IMP.core
import IMP.atom
import IMP.algebra


class Tests(IMP.test.TestCase):

    def _project(self, x):
        m = IMP.Model()
        p = IMP.Particle(m)
        IMP.core.XYZR.setup_particle(
            p, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(x, 2.0, 2.0), 1.0))
        IMP.atom.Mass.setup_particle(p, 1.0)
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(8, 8, 8))
        h = IMP.em.create_density_header(bb, 1.0)
        h.set_resolution(3.)
        s = IMP.em.SampledDensityMap(h)
        s.project([p], 1, 1, 1, IMP.algebra.Vector3D(0, 0, 0),
                  IMP.atom.Mass.get_mass_key())
        nx = s.get_header().get_nx()
        ny = s.get_header().get_ny()
        return dict(((i % nx, (i // nx) % ny, i // (nx * ny)), s.get_value(i))
                    for i in range(s.get_number_of_voxels())
                    if abs(s.get_value(i)) > 1e-9)

    def test_project_weights(self):
        """Check that project uses trilinear weights in [0,1]"""
        for x, expected in ((2.7, {(2, 2, 2): 0.3, (3, 2, 2): 0.7}),
                            (2.2, {(2, 2, 2): 0.8, (3, 2, 2): 0.2}),
                            (3.0, {(3, 2, 2): 1.0})):
            vals = self._project(x)
            self.assertEqual(sorted(vals.keys()), sorted(expected.keys()))
            for k, v in expected.items():
                self.assertAlmostEqual(vals[k], v, delta=1e-5)
            self.assertAlmostEqual(sum(vals.values()), 1.0, delta=1e-5)


if __name__ == '__main__':
    IMP.test.main()
