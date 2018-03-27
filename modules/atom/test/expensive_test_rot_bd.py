from __future__ import print_function
import IMP
import IMP.test
import IMP.container
import IMP.atom
import math

radius = 5


class Tests(IMP.test.TestCase):

    def _do_test_rot(self, dt):
        """Check rigid body brownian dynamics correlation time"""
        print("Testing for dt=", dt)
        m = IMP.Model()
        p = IMP.Particle(m)
        d = IMP.core.XYZR.setup_particle(p)
        d.set_radius(10)
        rb = IMP.core.RigidBody.setup_particle(
            p, IMP.algebra.ReferenceFrame3D())
        rb.set_coordinates_are_optimized(True)
        dd = IMP.atom.RigidBodyDiffusion.setup_particle(p)
        nD = dd.get_rotational_diffusion_coefficient()
        dd.set_rotational_diffusion_coefficient(nD)
        # print dd.get_rotational_diffusion_coefficient(),
        # dd.get_diffusion_coefficient()
        bd = IMP.atom.BrownianDynamics(m)
        bd.set_scoring_function([])
        bd.set_maximum_time_step(dt)
        angles = []
        rots = []
        for i in range(0, 3000):
            bd.optimize(1)
            rots.append(
                rb.get_rotation())
            angle = IMP.algebra.get_axis_and_angle(rots[-1])
            angles.append(angle[1])
        # print angles
        real_D= dd.get_rotational_diffusion_coefficient()
        IMP.set_log_level(IMP.PROGRESS)
        estimate_D= IMP.atom.get_rotational_diffusion_coefficient(rots, dt)
        print("Mean rot angle per dt/real rotD/estimated rotD: ",
              IMP.atom.get_diffusion_angle(real_D, dt),
              real_D, estimate_D)
        self.assertAlmostEqual(real_D, estimate_D, delta= .05*(real_D+estimate_D))
        return estimate_D


    def test_rot(self):
        """Check rigid body brownian dynamics correlation time"""
        estimate1000= self._do_test_rot(1000)
#        self._do_test_rot(100000)
        estimate10= self._do_test_rot(10)
        # verify numerical stability and robustness to step size
        self.assertAlmostEqual(estimate1000,
                               estimate10,
                               delta= .05*(estimate10+estimate1000))

if __name__ == '__main__':
    IMP.test.main()
