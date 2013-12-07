import IMP.kernel
import IMP.test
import IMP.atom
import IMP.core
import IMP.algebra
import IMP.multifit
import os


def read_surface(fname):
    for line in open(fname):
        spl = line.split()
        yield [int(x) for x in spl[:3]] + [float(x) for x in spl[3:]]


class Tests(IMP.test.TestCase):

    def test_single_atom(self):
        """Test generation of Connolly surface around a single atom"""
        m = IMP.kernel.Model()
        p = IMP.kernel.Particle(m)
        res = IMP.atom.Residue.setup_particle(p, IMP.atom.ALA)
        p = IMP.kernel.Particle(m)
        center = IMP.algebra.Vector3D(10., 10., 10.)
        IMP.core.XYZ.setup_particle(p, center)
        IMP.core.XYZR(p).set_radius(2.265)
        a = IMP.atom.Atom.setup_particle(p, IMP.atom.AT_CA)
        # Connolly radius of ALA:CA = 2.265A
        res.add_child(a)
        name = self.get_tmp_file_name("test.ms")
        IMP.multifit.write_connolly_surface([a], name, 5.0, 1.8)
        total_area = 0.
        for line in read_surface(name):
            at1, at2, at3, x, y, z, area, normx, normy, normz, half = line
            # Only one atom, so only convex surface
            self.assertEqual(at1, 1)
            self.assertEqual(at2, 0)
            self.assertEqual(at3, 0)
            total_area += area
            s = IMP.algebra.Vector3D(x, y, z)
            # Surface points should all lie on the surface of the atom
            self.assertAlmostEqual(IMP.algebra.get_squared_distance(s, center),
                                   2.265 * 2.265, delta=1e-2)
            # Surface normal should be a unit vector
            norm = IMP.algebra.Vector3D(normx, normy, normz)
            self.assertAlmostEqual(norm.get_squared_magnitude(), 1.0,
                                   delta=1e-2)
            # Last column should always be 0.5
            self.assertAlmostEqual(half, 0.5, delta=1e-3)
        # Total area should be roughly 4 * pi * r * r (r=2.265), but a little
        # less due to the probe not being pointlike
        self.assertAlmostEqual(total_area, 64.4, delta=0.1)


if __name__ == '__main__':
    IMP.test.main()
