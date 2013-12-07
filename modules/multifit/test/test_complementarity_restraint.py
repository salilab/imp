import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit


class Tests(IMP.test.TestCase):

    def _setup(self):
        """Set up two simple rigid bodies (spheres) and a restraint"""
        m = IMP.kernel.Model()
        sph = IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0., 0., 0.), 10.0)
        p0 = IMP.kernel.Particle(m)
        IMP.core.XYZR.setup_particle(p0, sph)
        IMP.atom.Mass.setup_particle(p0, 1.0)
        p1 = IMP.kernel.Particle(m)
        IMP.core.XYZR.setup_particle(p1, sph)
        IMP.atom.Mass.setup_particle(p1, 1.0)
        rb0 = IMP.core.RigidBody.setup_particle(IMP.kernel.Particle(m), [p0])
        rb1 = IMP.core.RigidBody.setup_particle(IMP.kernel.Particle(m), [p1])

        r = IMP.multifit.ComplementarityRestraint([p0], [p1])
        r.set_maximum_separation(30)
        r.set_complementarity_value(-10)
        r.set_maximum_penetration_score(30)
        r.set_interior_layer_thickness(5)
        r.set_complementarity_thickness(5)

        m.add_restraint(r)
        return (m, r, rb0, rb1)

    def _set_separation(self, rb, separation):
        tr = IMP.algebra.Transformation3D(
            IMP.algebra.Vector3D(separation, 0, 0))
        nrf = IMP.algebra.ReferenceFrame3D(tr)
        rb.set_reference_frame(nrf)

    def test_complementarity_restraint(self):
        """Check that ComplementarityRestraint works"""
        m, r, rb0, rb1 = self._setup()

        # Score when separation > max should be infinity:
        self._set_separation(rb0, 80.0)
        self.assertGreater(m.evaluate(False), 1e9)

        # Score when penetration > max should be infinity:
        self._set_separation(rb0, 18.0)
        self.assertGreater(m.evaluate(False), 1e9)

        # Otherwise, both complementarity and boundary components should
        # fall off with distance
        r.set_complementarity_coefficient(10.0)
        r.set_boundary_coefficient(0.0)
        self._set_separation(rb0, 20.0)
        self.assertAlmostEqual(m.evaluate(False), -72000., delta=1000.)
        # No complementarity contribution when
        # sphere distance > complementarity_thickness
        self._set_separation(rb0, 26.0)
        self.assertAlmostEqual(m.evaluate(False), 0.0, delta=1e-4)

        r.set_complementarity_coefficient(0.0)
        r.set_boundary_coefficient(10.0)
        self._set_separation(rb0, 20.0)
        self.assertAlmostEqual(m.evaluate(False), 625000., delta=1000.)
        self._set_separation(rb0, 26.0)
        self.assertAlmostEqual(m.evaluate(False), 359375., delta=1000.)

if __name__ == '__main__':
    IMP.test.main()
