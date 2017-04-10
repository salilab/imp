import math
import random

import IMP
import IMP.algebra
import IMP.atom
import IMP.core
import IMP.pmi.restraints.stereochemistry
import IMP.test


def _create_rigid_body(m):
    ps = [IMP.core.XYZ.setup_particle(IMP.Particle(m)) for i in range(3)]
    ps[0].set_coordinates(IMP.algebra.Vector3D(0, 0, 0))
    ps[1].set_coordinates(IMP.algebra.Vector3D(1, 0, 0))
    ps[2].set_coordinates(IMP.algebra.Vector3D(0, 1, 0))
    rb = IMP.core.RigidBody.setup_particle(IMP.Particle(m),
                                           IMP.algebra.ReferenceFrame3D())
    for p in ps:
        rb.add_member(p)
    return rb, ps


def _reference_frame_from_rot(offset=1., angle=0.):
    v = IMP.algebra.Vector3D(0, 0, 1)
    rot = IMP.algebra.get_rotation_about_axis(v, angle)
    trans = IMP.algebra.Transformation3D(rot, offset * v)
    return IMP.algebra.ReferenceFrame3D(trans)


class Tests(IMP.test.TestCase):

    def test_particles_values(self):
        """Test that random angles produce expected score."""
        m = IMP.Model()
        rb1, ps1 = _create_rigid_body(m)
        rb2, ps2 = _create_rigid_body(m)
        r = IMP.pmi.restraints.stereochemistry.PlaneDihedralRestraint(
                [ps1, ps2], angle=0., k=1.)
        for i in range(100):
            angle = random.uniform(-math.pi, math.pi)
            rf = _reference_frame_from_rot(1., angle)
            rb2.set_reference_frame(rf)
            self.assertAlmostEqual(r.evaluate(), (1 - math.cos(angle)))

    def test_optimization(self):
        """Test that optimization of dimer produces target dihedrals."""
        m = IMP.Model()
        rb1, ps1 = _create_rigid_body(m)
        rb2, ps2 = _create_rigid_body(m)
        rb2.set_coordinates_are_optimized(True)

        for i in range(10):
            target_angle = random.uniform(-math.pi, math.pi)
            r = IMP.pmi.restraints.stereochemistry.PlaneDihedralRestraint(
                    [ps1, ps2], angle=target_angle * 180 / math.pi, k=1.)
            sf = IMP.core.RestraintsScoringFunction(r.rs)

            rf = _reference_frame_from_rot(
                5., random.uniform(-math.pi, math.pi))
            rb2.set_reference_frame(rf)

            opt = IMP.core.SteepestDescent(m)
            opt.set_scoring_function(sf)
            opt.optimize(100)
            self.assertAlmostEqual(r.evaluate(), 0, delta=1e-6)
            dihed1 = IMP.core.get_dihedral(ps1[1], ps1[0], ps2[0], ps2[1])
            dihed2 = IMP.core.get_dihedral(ps1[2], ps1[0], ps2[0], ps2[2])
            self.assertAlmostEqual(dihed1, target_angle, delta=1e-3)
            self.assertAlmostEqual(dihed2, target_angle, delta=1e-3)

    def test_optimization_string(self):
        """Test that optimization of long string produces target dihedrals."""
        m = IMP.Model()
        rbs, pss = zip(*[_create_rigid_body(m) for i in range(5)])

        target_angle = math.pi / 2.
        r = IMP.pmi.restraints.stereochemistry.PlaneDihedralRestraint(
                pss, angle=target_angle * 180 / math.pi, k=1.)
        sf = IMP.core.RestraintsScoringFunction(r.rs)

        for i, rb in enumerate(rbs[1:]):
            rb.set_coordinates_are_optimized(True)
            rf = _reference_frame_from_rot(
                5. * (i + 1), random.uniform(-math.pi, 0.))
            rb.set_reference_frame(rf)

        opt = IMP.core.SteepestDescent(m)
        opt.set_scoring_function(sf)
        opt.optimize(500)
        self.assertAlmostEqual(r.evaluate(), 0., delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
