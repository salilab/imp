import math
import IMP
import IMP.test
import IMP.em
import IMP.core
import IMP.atom
import IMP.algebra


class Tests(IMP.test.TestCase):

    def test_density_from_particle(self):
        """Check bayesem3d_get_density_from_particle against analytic values"""
        m = IMP.Model()
        p = IMP.Particle(m)
        center = IMP.algebra.Vector3D(5, 5, 5)
        IMP.core.XYZR.setup_particle(p, IMP.algebra.Sphere3D(center, 1.0))
        IMP.atom.Mass.setup_particle(p, 1e7)
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(10, 10, 10))
        em = IMP.em.create_density_map(bb, 1.0)
        resolution = 3.0
        out = IMP.em.bayesem3d_get_density_from_particle(em, [p], resolution,
                                                         1.0)
        kp = IMP.em.KernelParameters(resolution)
        h = out.get_header()
        nchecked = 0
        for z in range(h.get_nz()):
            for y in range(h.get_ny()):
                for x in range(h.get_nx()):
                    v = out.xyz_ind2voxel(x, y, z)
                    # Voxels outside of the particle's local box are zero
                    if out.get_value(v) == 0.:
                        continue
                    nchecked += 1
                    c = out.get_location_by_voxel(v)
                    r2 = IMP.algebra.get_squared_distance(c, center)
                    expected = 1e7 * kp.get_rnormfac() * math.exp(
                        -r2 * kp.get_inv_rsigsq())
                    self.assertAlmostEqual(out.get_value(v), expected,
                                           delta=1e-4 * expected + 1e-6)
        self.assertGreater(nchecked, 100)


if __name__ == '__main__':
    IMP.test.main()
