import IMP
import IMP.test
import IMP.container
import IMP.atom
import math

radius=5

class Tests(IMP.test.TestCase):

    def _do_test_rot(self, dt):
        """Check rigid body brownian dynamics correlation time"""
        m= IMP.Model()
        p =IMP.kernel.Particle(m)
        d=IMP.core.XYZR.setup_particle(p)
        d.set_radius(10)
        rb= IMP.core.RigidBody.setup_particle(p, IMP.algebra.ReferenceFrame3D())
        rb.set_coordinates_are_optimized(True)
        dd= IMP.atom.RigidBodyDiffusion.setup_particle(p)
        nD=dd.get_rotational_diffusion_coefficient()
        dd.set_rotational_diffusion_coefficient(nD)
        #print dd.get_rotational_diffusion_coefficient(), dd.get_diffusion_coefficient()
        bd= IMP.atom.BrownianDynamics(m)
        bd.set_maximum_time_step(dt)
        angles=[]
        rots=[]
        for i in range(0, 1000):
            bd.optimize(1)
            rots.append(rb.get_reference_frame().get_transformation_to().get_rotation())
            angle= IMP.algebra.get_axis_and_angle(rots[-1])
            angles.append(angle[1])
        #print angles
        print IMP.atom.get_diffusion_angle(dd.get_rotational_diffusion_coefficient(), dt),\
          dd.get_rotational_diffusion_coefficient(),\
          IMP.atom.get_rotational_diffusion_coefficient(rots, dt)
        self.assertAlmostEqual(dd.get_rotational_diffusion_coefficient(),
                               IMP.atom.get_rotational_diffusion_coefficient(rots, dt),
                               delta=.5*dd.get_rotational_diffusion_coefficient())
    def test_rot(self):
        """Check rigid body brownian dynamics correlation time"""
        #self._do_test_rot(10000)
        #loose steps to numerical errors
        self._do_test_rot(1000)
        #self._do_test_rot(100)
        #self._do_test_rot(10)
if __name__ == '__main__':
    IMP.test.main()
