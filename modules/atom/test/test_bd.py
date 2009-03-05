import unittest
import IMP, IMP.test
import IMP.core
import IMP.atom



class MCOptimizerTest(IMP.test.TestCase):
    def _compute_moments(self, ps):
        m= IMP.algebra.Vector3D(0,0,0)
        for p in ps:
            d= IMP.core.XYZDecorator(p)
            m+= d.get_coordinates()
        m=m/len(ps)
        s= IMP.algebra.Vector3D(0,0,0)
        for p in ps:
            d= IMP.core.XYZDecorator(p)
            diff= d.get_coordinates()-m
            for i in range(0,3):
                s[i]= s[i]+diff[i]*diff[i]
        s=s/len(ps)
        for i in range(0,3):
            s[i]= s[i]**.5
        return (m[0], m[1], m[2], s[0], s[1], s[2])
    def test_step_size(self):
        """Testing the step size invariance of free diffusion"""
        m= IMP.Model()
        bd= IMP.atom.BrownianDynamics()
        bd.set_model(m)
        radius=2
        ps= IMP.core.create_xyzr_particles(m, 400, radius)
        for p in ps:
            d= IMP.atom.DiffusionDecorator.create(p)
            d.set_D_from_radius_in_angstroms(radius)
            d.set_coordinates_are_optimized(True)
            d.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        bd.set_time_step_in_femtoseconds(1e4)
        IMP.set_log_level(IMP.SILENT)
        bd.optimize(1000)
        moments= self._compute_moments(ps)
        print moments
        for p in ps:
            d= IMP.atom.DiffusionDecorator(p)
            d.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        bd.set_time_step_in_femtoseconds(1e7)
        bd.optimize(1)
        big_moments= self._compute_moments(ps)
        print big_moments
        for i in range(0, len(moments)):
            self.assertInTolerance(big_moments[i], moments[i],
                                   .1*(big_moments[i]+ moments[i])+30)

if __name__ == '__main__':
    unittest.main()
