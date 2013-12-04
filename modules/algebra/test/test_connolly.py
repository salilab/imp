import IMP
import IMP.test
import IMP.atom
import IMP.core
import IMP.algebra
import os


class Tests(IMP.test.TestCase):

    def test_single_atom(self):
        """Test generation of Connolly surface around a single atom"""
        m = IMP.kernel.Model()
        p = IMP.kernel.Particle(m)
        res = IMP.atom.Residue.setup_particle(p, IMP.atom.ALA)
        p = IMP.kernel.Particle(m)
        center = IMP.algebra.Vector3D(10., 10., 10.)
        sphere = IMP.algebra.Sphere3D(center, 2.265)
        pts = IMP.algebra.get_connolly_surface([sphere], 5.0, 1.8)
        total_area = 0.
        for sp in pts:
            # Only one atom, so only convex surface
            self.assertEqual(sp.atom_0, 0)
            self.assertEqual(sp.atom_1, -1)
            self.assertEqual(sp.atom_2, -1)
            total_area += sp.area
            # Surface points should all lie on the surface of the atom
            self.assertAlmostEqual(
                IMP.algebra.get_distance(sp.surface_point, center),
                2.265, delta=1e-2)
            # Surface normal should be a unit vector
            self.assertAlmostEqual(sp.normal.get_squared_magnitude(), 1.0,
                                   delta=1e-2)
        # Total area should be roughly 4 * pi * r * r (r=2.265), but a little
        # less due to the probe not being pointlike
        self.assertAlmostEqual(total_area, 64.4, delta=0.1)

    def test_molecule(self):
        m = IMP.kernel.Model()
        name = self.get_input_file_name("input.pdb")
        pdb = IMP.kernel._create_particles_from_pdb(name, m)
        spheres = [IMP.core.XYZR(p).get_sphere() for p in pdb]
        sps = IMP.algebra.get_connolly_surface(spheres, 5, 1.8)

        sps_area = sum([s.area for s in sps])
        self.assertAlmostEqual(sps_area, 5477.4, delta=.2)

if __name__ == '__main__':
    IMP.test.main()
